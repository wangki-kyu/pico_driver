/*++

Module Name:

    queue.c

Abstract:

    This file contains the queue entry points and callbacks.

Environment:

    Kernel-mode Driver Framework

--*/

#include "driver.h"
#include "queue.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, picodriverQueueInitialize)
#endif

void DumpBuffer(WDFMEMORY hMem, size_t len) {
    size_t size;
    PUCHAR p = (PUCHAR)WdfMemoryGetBuffer(hMem, &size);
    for (size_t i = 0; i < len && i < size; i++) DbgPrint("%02X ", p[i]);
    DbgPrint("\n");
}

NTSTATUS
picodriverQueueInitialize(
    _In_ WDFDEVICE Device
    )
/*++

Routine Description:


     The I/O dispatch callbacks for the frameworks device object
     are configured in this function.

     A single default I/O Queue is configured for parallel request
     processing, and a driver context memory allocation is created
     to hold our structure QUEUE_CONTEXT.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    VOID

--*/
{
    WDFQUEUE queue;
    NTSTATUS status;
    WDF_IO_QUEUE_CONFIG    queueConfig;

    PAGED_CODE();
    
    //
    // Configure a default queue so that requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching to goto
    // other queues get dispatched here.
    //
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
         &queueConfig,
        WdfIoQueueDispatchParallel
        );

    queueConfig.EvtIoDeviceControl = picodriverEvtIoDeviceControl;
    queueConfig.EvtIoStop = picodriverEvtIoStop;

    status = WdfIoQueueCreate(
                 Device,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if( !NT_SUCCESS(status) ) {
        DbgPrint("WdfIoQueueCreate failed 0x%x\n", status);
        return status;
    }

    return status;
}

VOID
picodriverEvtIoDeviceControl(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t OutputBufferLength,
    _In_ size_t InputBufferLength,
    _In_ ULONG IoControlCode
    )
/*++

Routine Description:

    This event is invoked when the framework receives IRP_MJ_DEVICE_CONTROL request.

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    OutputBufferLength - Size of the output buffer in bytes

    InputBufferLength - Size of the input buffer in bytes

    IoControlCode - I/O control code.

Return Value:

    VOID

--*/
{
    DbgPrint("[%s] Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode 0x%x\n",
                __FUNCTION__, Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);

    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(device);
    NTSTATUS status = STATUS_SUCCESS;

    switch (IoControlCode) {
        case IOCTL_PICO_TEST_WRITE: {
            DbgPrint("IOCTL_PICO_TEST_WRITE received\n");

            // Validate that the write pipe is available
            if (pDeviceContext->WritePipe == NULL) {
                DbgPrint("ERROR: WritePipe is NULL - device not ready for writing\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            // Retrieve the input buffer memory handle from the IOCTL request
            // This memory contains the data to be sent to the Pico device
            WDFMEMORY memoryHandle = NULL;
            status = WdfRequestRetrieveInputMemory(Request, &memoryHandle);

            DumpBuffer(memoryHandle, InputBufferLength);

            if (!NT_SUCCESS(status)) {
                DbgPrint("ERROR: WdfRequestRetrieveInputMemory failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            // Format the WDF request for bulk write operation
            // This prepares the request to be sent to the USB device via the write pipe
            // The memory handle specifies where the data to be written is located
            status = WdfUsbTargetPipeFormatRequestForWrite(
                pDeviceContext->WritePipe,
                Request,
                memoryHandle,
                NULL  // No additional memory offset needed
            );

            if (!NT_SUCCESS(status)) {
                DbgPrint("ERROR: WdfUsbTargetPipeFormatRequestForWrite failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("Request formatted for write operation (%d bytes)\n", (int)InputBufferLength);

            // Actually send the formatted request to the USB device
            // WDF_REQUEST_SEND_OPTION_SYNCHRONOUS means this call will wait for completion
            WDF_REQUEST_SEND_OPTIONS sendOptions;
            WDF_REQUEST_SEND_OPTIONS_INIT(&sendOptions, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);

            if (!WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pDeviceContext->WritePipe), &sendOptions)) {
                // If WdfRequestSend returns FALSE, retrieve the completion status
                status = WdfRequestGetStatus(Request);
                DbgPrint("ERROR: WdfRequestSend failed with status 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            // Request was successfully sent and completed (synchronous mode)
            DbgPrint("Write request successfully sent to device\n");
            return;
        }

        case IOCTL_PICO_TEST_READ: {
            DbgPrint("IOCTL_PICO_TEST_READ received\n");

            // Validate that the read pipe is available
            if (pDeviceContext->ReadPipe == NULL) {
                DbgPrint("ERROR: ReadPipe is NULL - device not ready for reading\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            // Retrieve the output buffer memory handle from the IOCTL request
            // This memory will receive the data from the Pico device
            WDFMEMORY memoryHandle = NULL;
            status = WdfRequestRetrieveOutputMemory(Request, &memoryHandle);

            if (!NT_SUCCESS(status)) {
                DbgPrint("ERROR: WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            // Format the WDF request for bulk read operation
            // This prepares the request to receive data from the USB device via the read pipe
            // The memory handle specifies where the received data will be placed
            status = WdfUsbTargetPipeFormatRequestForRead(
                pDeviceContext->ReadPipe,
                Request,
                memoryHandle,
                NULL  // No additional memory offset needed
            );

            if (!NT_SUCCESS(status)) {
                DbgPrint("ERROR: WdfUsbTargetPipeFormatRequestForRead failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("Request formatted for read operation (max %d bytes)\n", (int)OutputBufferLength);

            // Actually send the formatted request to the USB device
            // WDF_REQUEST_SEND_OPTION_SYNCHRONOUS means this call will wait for completion
            WDF_REQUEST_SEND_OPTIONS sendOptions;
            WDF_REQUEST_SEND_OPTIONS_INIT(&sendOptions, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);

            if (!WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pDeviceContext->ReadPipe), &sendOptions)) {
                // If WdfRequestSend returns FALSE, retrieve the completion status
                status = WdfRequestGetStatus(Request);
                DbgPrint("ERROR: WdfRequestSend failed with status 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            // Request was successfully sent and completed (synchronous mode)
            DbgPrint("Read request successfully sent to device\n");
            return;
        }

        default:
            DbgPrint("Unknown IOCTL code: 0x%x\n", IoControlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            WdfRequestComplete(Request, status);
            return;
    }
    return;
}

VOID
picodriverEvtIoStop(
    _In_ WDFQUEUE Queue,
    _In_ WDFREQUEST Request,
    _In_ ULONG ActionFlags
)
/*++

Routine Description:

    This event is invoked for a power-managed queue before the device leaves the working state (D0).

Arguments:

    Queue -  Handle to the framework queue object that is associated with the
             I/O request.

    Request - Handle to a framework request object.

    ActionFlags - A bitwise OR of one or more WDF_REQUEST_STOP_ACTION_FLAGS-typed flags
                  that identify the reason that the callback function is being called
                  and whether the request is cancelable.

Return Value:

    VOID

--*/
{
    DbgPrint("[%s] Queue 0x%p, Request 0x%p ActionFlags %d\n",
                __FUNCTION__, Queue, Request, ActionFlags);

    //
    // In most cases, the EvtIoStop callback function completes, cancels, or postpones
    // further processing of the I/O request.
    //
    // Typically, the driver uses the following rules:
    //
    // - If the driver owns the I/O request, it either postpones further processing
    //   of the request and calls WdfRequestStopAcknowledge, or it calls WdfRequestComplete
    //   with a completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
    //  
    //   The driver must call WdfRequestComplete only once, to either complete or cancel
    //   the request. To ensure that another thread does not call WdfRequestComplete
    //   for the same request, the EvtIoStop callback must synchronize with the driver's
    //   other event callback functions, for instance by using interlocked operations.
    //
    // - If the driver has forwarded the I/O request to an I/O target, it either calls
    //   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
    //   further processing of the request and calls WdfRequestStopAcknowledge.
    //
    // A driver might choose to take no action in EvtIoStop for requests that are
    // guaranteed to complete in a small amount of time. For example, the driver might
    // take no action for requests that are completed in one of the driver�s request handlers.
    //

    return;
}
