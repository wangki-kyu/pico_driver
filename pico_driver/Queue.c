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

VOID
picodriverEvtInterruptReadComplete(
    _In_ WDFUSBPIPE Pipe,
    _In_ WDFMEMORY Buffer,
    _In_ size_t NumBytesTransferred,
    _In_ WDFCONTEXT Context
    )
/*++

Routine Description:

    Completion callback for interrupt endpoint continuous read.
    WDF automatically handles request resubmission.
    We just need to process the received interrupt data.

Arguments:

    Pipe - The USB pipe

    Buffer - The data buffer from continuous reader

    NumBytesTransferred - Number of bytes in the buffer

    Context - Device context

Return Value:

    VOID

--*/
{
    UNREFERENCED_PARAMETER(Pipe);
    UNREFERENCED_PARAMETER(Context);

    if (NumBytesTransferred > 0) {
        // Process interrupt data
        PUCHAR pData = (PUCHAR)WdfMemoryGetBuffer(Buffer, NULL);

        DbgPrint("[picodriverEvtInterruptReadComplete] Interrupt data received (%zu bytes):\n", NumBytesTransferred);
        for (size_t i = 0; i < NumBytesTransferred && i < 32; i++) {
            if (i % 16 == 0) {
                DbgPrint("[interrupt] %04lX: ", (ULONG)i);
            }
            DbgPrint("%02X ", pData[i]);
            if ((i + 1) % 16 == 0) {
                DbgPrint("\n");
            }
        }
        if (NumBytesTransferred % 16 != 0) {
            DbgPrint("\n");
        }
    }

    // No need to resubmit - WDF continuous reader handles it automatically
}

VOID
PicoDmaWriteComplete(
    _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target,
    _In_ PWDF_REQUEST_COMPLETION_PARAMS Params,
    _In_opt_ WDFCONTEXT Context
    )
{
    UNREFERENCED_PARAMETER(Target);
    UNREFERENCED_PARAMETER(Context);

    NTSTATUS status = Params->IoStatus.Status;
    ULONG_PTR bytesWritten = Params->IoStatus.Information;

    DbgPrint("[pico_driver] DMA Write complete: status 0x%x, bytes %llu\n", status, (ULONGLONG)bytesWritten);
    WdfRequestComplete(Request, status);
}

void DumpBuffer(WDFMEMORY hMem, size_t len) {
    size_t size;
    PUCHAR p = (PUCHAR)WdfMemoryGetBuffer(hMem, &size);
    for (size_t i = 0; i < len && i < size; i++) DbgPrint("[pico_driver] %02X ", p[i]);
    DbgPrint("[pico_driver] \n");
}

// Context structure for inference two-phase operation (write + read)
typedef struct {
    WDFREQUEST OriginalRequest;
    WDFMEMORY OutputMemory;
    PDEVICE_CONTEXT DeviceContext;
} INFERENCE_CONTEXT, *PINFERENCE_CONTEXT;

VOID
PicoInferenceReadComplete(
    _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target,
    _In_ PWDF_REQUEST_COMPLETION_PARAMS Params,
    _In_opt_ WDFCONTEXT Context
    )
{
    UNREFERENCED_PARAMETER(Target);
    UNREFERENCED_PARAMETER(Context);

    //PINFERENCE_CONTEXT infContext = (PINFERENCE_CONTEXT)Context;
    NTSTATUS status = Params->IoStatus.Status;
    ULONG_PTR bytesRead = Params->IoStatus.Information;

    if (status == STATUS_IO_TIMEOUT) {
        DbgPrint("[pico_driver] Inference Read timed out! Device is not responding\n");
        WdfRequestComplete(Request, status);
        return;
    }

    DbgPrint("[pico_driver] Inference Read complete: status 0x%x, bytes read %llu\n", status, (ULONGLONG)bytesRead);

    if (NT_SUCCESS(status)) {
        // Get output memory to access the read data
        WDFMEMORY outputMemory = NULL;
        NTSTATUS retrieveStatus = WdfRequestRetrieveOutputMemory(Request, &outputMemory);

        if (NT_SUCCESS(retrieveStatus)) {
            // Get buffer pointer
            PUCHAR pData = (PUCHAR)WdfMemoryGetBuffer(outputMemory, NULL);

            // Log the read data in hex format (16 bytes per line)
            DbgPrint("[pico_driver] Inference result data (%llu bytes):\n", (ULONGLONG)bytesRead);
            for (ULONG_PTR i = 0; i < bytesRead && i < 64; i++) {
                if (i % 16 == 0) {
                    DbgPrint("[pico_driver] %04llX: ", (ULONGLONG)i);
                }
                DbgPrint("%02X ", pData[i]);
                if ((i + 1) % 16 == 0) {
                    DbgPrint("\n");
                }
            }
            if (bytesRead % 16 != 0) {
                DbgPrint("\n");
            }
        }

        // Set the number of bytes returned to client
        WdfRequestSetInformation(Request, bytesRead);
    }

    // Complete the original request with the result
    WdfRequestComplete(Request, status);

    // Free the temporary read request
    /*WdfObjectDelete(Request);*/

    // Free context
    /*ExFreePool(infContext);*/
}

VOID
PicoInferenceWriteComplete(
    _In_ WDFREQUEST Request,
    _In_ WDFIOTARGET Target,
    _In_ PWDF_REQUEST_COMPLETION_PARAMS Params,
    _In_opt_ WDFCONTEXT Context
    )
{
    UNREFERENCED_PARAMETER(Target);
    UNREFERENCED_PARAMETER(Context);

    //PINFERENCE_CONTEXT infContext = (PINFERENCE_CONTEXT)Context;
    NTSTATUS status = Params->IoStatus.Status;
    ULONG_PTR bytesWritten = Params->IoStatus.Information;

    DbgPrint("[pico_driver] Inference Write complete: status 0x%x, bytes written %llu\n", status, (ULONGLONG)bytesWritten);

    if (!NT_SUCCESS(status)) {
        DbgPrint("[pico_driver] ERROR: Inference write failed, completing request with error\n");
        WdfRequestComplete(Request, status);
        //ExFreePool(infContext);
        return;
    }

    // Now read the inference result from device
    DbgPrint("[pico_driver] Inference write successful, now reading result from device...\n");

    // Create a new request for reading the result
    //WDFREQUEST readRequest = NULL;
    //WDF_OBJECT_ATTRIBUTES requestAttributes;
    //WDF_OBJECT_ATTRIBUTES_INIT(&requestAttributes);
    //requestAttributes.ParentObject = infContext->DeviceContext->WritePipe;  // Auto-cleanup when device closes

    //status = WdfRequestCreate(&requestAttributes, WdfUsbTargetPipeGetIoTarget(infContext->DeviceContext->ReadPipe), &readRequest);
    //if (!NT_SUCCESS(status)) {
    //    DbgPrint("[pico_driver] ERROR: Failed to create read request 0x%x\n", status);
    //    WdfRequestComplete(infContext->OriginalRequest, status);
    //    ExFreePool(infContext);
    //    return;
    //}

  /*  WDF_REQUEST_REUSE_PARAMS reuseParams;
    WDF_REQUEST_REUSE_PARAMS_INIT(&reuseParams, WDF_REQUEST_REUSE_SET_NEW_IRP, STATUS_SUCCESS);
    WdfRequestReuse(Request, &reuseParams);*/

    WDFQUEUE queue = WdfRequestGetIoQueue(Request);
    WDFDEVICE device = WdfIoQueueGetDevice(queue);
    PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(device);
    WDFUSBPIPE readPipe = pDeviceContext->ReadPipe;

    // OutputBuffer 가져오기 
    WDFMEMORY outputMemory = NULL;
    status = WdfRequestRetrieveOutputMemory(Request, &outputMemory);

    if (!NT_SUCCESS(status)) {
        DbgPrint("[pico_driver] ERROR: WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
        WdfRequestComplete(Request, status);
        return;
    }

    // Format the read request
    status = WdfUsbTargetPipeFormatRequestForRead(
        readPipe,
        Request,
        outputMemory,
        NULL
    );

    // reference count decrease 
    /*WdfObjectDereference(outputMemory);*/

    if (!NT_SUCCESS(status)) {
        DbgPrint("[pico_driver] ERROR: WdfUsbTargetPipeFormatRequestForRead failed 0x%x\n", status);
        WdfRequestComplete(Request, status);
        return;
    }

    // Set completion routine for read
    WdfRequestSetCompletionRoutine(Request, PicoInferenceReadComplete, NULL);

    WDF_REQUEST_SEND_OPTIONS sendOptions;
    WDF_REQUEST_SEND_OPTIONS_INIT(&sendOptions, 0);
    WDF_REQUEST_SEND_OPTIONS_SET_TIMEOUT(&sendOptions, WDF_REL_TIMEOUT_IN_MS(30000));

    // Send read request asynchronously
    if (!WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(readPipe), &sendOptions)) {
        status = WdfRequestGetStatus(Request);
        DbgPrint("[pico_driver] ERROR: WdfRequestSend (read) failed 0x%x\n", status);
        WdfRequestComplete(Request, status);
        return;
    }

    DbgPrint("[pico_driver] Inference read request sent successfully\n");
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
        DbgPrint("[pico_driver] WdfIoQueueCreate failed 0x%x\n", status);
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
    DbgPrint("[pico_driver][%s] Queue 0x%p, Request 0x%p OutputBufferLength %d InputBufferLength %d IoControlCode 0x%x\n",
                __FUNCTION__, Queue, Request, (int) OutputBufferLength, (int) InputBufferLength, IoControlCode);

    WDFDEVICE device = WdfIoQueueGetDevice(Queue);
    PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(device);
    NTSTATUS status = STATUS_SUCCESS;

    switch (IoControlCode) {
        case IOCTL_PICO_TEST_WRITE: {
            DbgPrint("[pico_driver] IOCTL_PICO_TEST_WRITE received\n");

            // Validate that the write pipe is available
            if (pDeviceContext->WritePipe == NULL) {
                DbgPrint("[pico_driver] ERROR: WritePipe is NULL - device not ready for writing\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            // Retrieve the input buffer
            PUCHAR inputBuffer = NULL;
            status = WdfRequestRetrieveInputBuffer(Request, InputBufferLength, (PVOID*)&inputBuffer, NULL);

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfRequestRetrieveInputBuffer failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Input buffer retrieved, %d bytes\n", (int)InputBufferLength);

            // Create memory descriptor
            WDF_MEMORY_DESCRIPTOR memDesc;
            WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDesc, inputBuffer, (ULONG)InputBufferLength);

            // Set up timeout options (3 seconds)
            WDF_REQUEST_SEND_OPTIONS sendOptions;
            WDF_REQUEST_SEND_OPTIONS_INIT(&sendOptions, 0);
            sendOptions.Timeout = WDF_REL_TIMEOUT_IN_MS(3000);

            // Send data synchronously
            ULONG bytesWritten = 0;
            DbgPrint("[pico_driver] Calling WdfUsbTargetPipeWriteSynchronously with 3sec timeout...\n");

            status = WdfUsbTargetPipeWriteSynchronously(
                pDeviceContext->WritePipe,
                WDF_NO_HANDLE,
                &sendOptions,  // 3sec timeout 
                &memDesc,
                &bytesWritten
            );

            DbgPrint("[pico_driver] WdfUsbTargetPipeWriteSynchronously returned status 0x%x, bytes written: %d\n", status, bytesWritten);

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfUsbTargetPipeWriteSynchronously failed 0x%x\n", status);
            } else {
                DbgPrint("[pico_driver] Write successful, sent %d bytes\n", bytesWritten);
            }

            break;
        }

        case IOCTL_PICO_TEST_READ: {
            DbgPrint("[pico_driver] IOCTL_PICO_TEST_READ received\n");

            // Validate that the read pipe is available
            if (pDeviceContext->ReadPipe == NULL) {
                DbgPrint("[pico_driver] ERROR: ReadPipe is NULL - device not ready for reading\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            // Retrieve the output buffer memory handle from the IOCTL request
            // This memory will receive the data from the Pico device
            WDFMEMORY memoryHandle = NULL;
            status = WdfRequestRetrieveOutputMemory(Request, &memoryHandle);

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
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
                DbgPrint("[pico_driver] ERROR: WdfUsbTargetPipeFormatRequestForRead failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Request formatted for read operation (max %d bytes)\n", (int)OutputBufferLength);

            // Actually send the formatted request to the USB device
            // WDF_REQUEST_SEND_OPTION_SYNCHRONOUS means this call will wait for completion
            WDF_REQUEST_SEND_OPTIONS sendOptions;
            WDF_REQUEST_SEND_OPTIONS_INIT(&sendOptions, WDF_REQUEST_SEND_OPTION_SYNCHRONOUS);

            if (!WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pDeviceContext->ReadPipe), &sendOptions)) {
                // If WdfRequestSend returns FALSE, retrieve the completion status
                status = WdfRequestGetStatus(Request);
                DbgPrint("[pico_driver] ERROR: WdfRequestSend failed with status 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            // Request was successfully sent and completed (synchronous mode)
            DbgPrint("[pico_driver] Read request successfully sent to device\n");
            break;
        }

        case IOCTL_PICO_READ_TEMP: {
            DbgPrint("[pico_driver] IOCTL_PICO_READ_TEMP received\n");

            // Validate that both pipes are available
            if (pDeviceContext->WritePipe == NULL) {
                DbgPrint("[pico_driver] ERROR: WritePipe is NULL - device not ready\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            if (pDeviceContext->ReadPipe == NULL) {
                DbgPrint("[pico_driver] ERROR: ReadPipe is NULL - device not ready\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            // Step 1: Send temperature read command (0x10) to the device
            UCHAR cmdByte = SENSOR_READ_TEMP;
            WDF_MEMORY_DESCRIPTOR memDescCmd;
            WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDescCmd, &cmdByte, sizeof(cmdByte));

            WDF_REQUEST_SEND_OPTIONS sendOptions;
            WDF_REQUEST_SEND_OPTIONS_INIT(&sendOptions, 0);
            sendOptions.Timeout = WDF_REL_TIMEOUT_IN_MS(3000);

            ULONG bytesWritten = 0;
            DbgPrint("[pico_driver] Sending temperature read command (0x%02x)...\n", cmdByte);

            status = WdfUsbTargetPipeWriteSynchronously(
                pDeviceContext->WritePipe,
                WDF_NO_HANDLE,
                &sendOptions,
                &memDescCmd,
                &bytesWritten
            );

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: Failed to send temperature command 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Temperature command sent successfully (%d bytes)\n", bytesWritten);

            // Step 2: Read temperature data from the device
            DbgPrint("[pico_driver] OutputBufferLength: %d bytes\n", (int)OutputBufferLength);

            if (OutputBufferLength == 0) {
                DbgPrint("[pico_driver] ERROR: OutputBufferLength is 0\n");
                status = STATUS_BUFFER_TOO_SMALL;
                WdfRequestComplete(Request, status);
                return;
            }

            WDFMEMORY memoryHandle = NULL;
            status = WdfRequestRetrieveOutputMemory(Request, &memoryHandle);

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            // Read temperature data directly from USB pipe
            WDF_MEMORY_DESCRIPTOR memDescRead;
            WDF_MEMORY_DESCRIPTOR_INIT_HANDLE(&memDescRead, memoryHandle, NULL);

            WDF_REQUEST_SEND_OPTIONS readOptions;
            WDF_REQUEST_SEND_OPTIONS_INIT(&readOptions, 0);
            readOptions.Timeout = WDF_REL_TIMEOUT_IN_MS(1000);

            ULONG bytesRead = 0;
            DbgPrint("[pico_driver] Reading temperature data from device...\n");

            status = WdfUsbTargetPipeReadSynchronously(
                pDeviceContext->ReadPipe,
                WDF_NO_HANDLE,
                &readOptions,
                &memDescRead,
                &bytesRead
            );

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfUsbTargetPipeReadSynchronously failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Temperature data read successfully, %d bytes received\n", bytesRead);

            // Set the information about how many bytes were read
            WdfRequestSetInformation(Request, bytesRead);
            break;
        }

        case IOCTL_PICO_DMA_WRITE: {
            DbgPrint("[pico_driver] IOCTL_PICO_DMA_WRITE received\n");

            if (pDeviceContext->WritePipe == NULL) {
                DbgPrint("[pico_driver] ERROR: WritePipe is NULL - device not ready for writing\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            // Retrieve input memory (DMA - no buffer copy, uses MDL)
            WDFMEMORY inputMemory = NULL;
            status = WdfRequestRetrieveInputMemory(Request, &inputMemory);

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfRequestRetrieveInputMemory failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Input memory retrieved for DMA write\n");

            // Format the request for USB pipe write (asynchronous)
            status = WdfUsbTargetPipeFormatRequestForWrite(
                pDeviceContext->WritePipe,
                Request,
                inputMemory,
                NULL  // No additional memory offset needed
            );

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfUsbTargetPipeFormatRequestForWrite failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Request formatted for DMA write operation\n");

            // Set completion routine to handle asynchronous completion
            WdfRequestSetCompletionRoutine(Request, PicoDmaWriteComplete, NULL);

            // Send the request asynchronously (DMA mode)
            if (!WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pDeviceContext->WritePipe), WDF_NO_SEND_OPTIONS)) {
                status = WdfRequestGetStatus(Request);
                DbgPrint("[pico_driver] ERROR: WdfRequestSend failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            // Asynchronous - completion will be handled by PicoDmaWriteComplete callback
            DbgPrint("[pico_driver] DMA write request sent successfully\n");
            return;
        }

        case IOCTL_PICO_MODEL_LOAD: {
            DbgPrint("[pico_driver] IOCTL_PICO_MODEL_LOAD received (size: %d bytes)\n", (int)InputBufferLength);

            if (pDeviceContext->WritePipe == NULL) {
                DbgPrint("[pico_driver] ERROR: WritePipe is NULL - device not ready for writing\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            // Retrieve input memory (DMA - no buffer copy, uses MDL)
            WDFMEMORY inputMemory = NULL;
            status = WdfRequestRetrieveInputMemory(Request, &inputMemory);

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfRequestRetrieveInputMemory failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Input memory retrieved for model load\n");

            // Format the request for USB pipe write (asynchronous)
            status = WdfUsbTargetPipeFormatRequestForWrite(
                pDeviceContext->WritePipe,
                Request,
                inputMemory,
                NULL
            );

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfUsbTargetPipeFormatRequestForWrite failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Request formatted for model load operation\n");

            // Set completion routine to handle asynchronous completion
            WdfRequestSetCompletionRoutine(Request, PicoDmaWriteComplete, NULL);

            // Send the request asynchronously
            if (!WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pDeviceContext->WritePipe), WDF_NO_SEND_OPTIONS)) {
                status = WdfRequestGetStatus(Request);
                DbgPrint("[pico_driver] ERROR: WdfRequestSend failed 0x%x\n", status);
                WdfRequestComplete(Request, status);
                return;
            }

            // Asynchronous - completion will be handled by PicoDmaWriteComplete callback
            DbgPrint("[pico_driver] Model load request sent successfully\n");
            return;
        }

        case IOCTL_PICO_RUN_INFERENCE: {
            DbgPrint("[pico_driver] IOCTL_PICO_RUN_INFERENCE received (input size: %d bytes, output size: %d bytes)\n",
                (int)InputBufferLength, (int)OutputBufferLength);

            if (pDeviceContext->WritePipe == NULL) {
                DbgPrint("[pico_driver] ERROR: WritePipe is NULL - device not ready for writing\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            if (pDeviceContext->ReadPipe == NULL) {
                DbgPrint("[pico_driver] ERROR: ReadPipe is NULL - device not ready for reading\n");
                status = STATUS_DEVICE_NOT_READY;
                WdfRequestComplete(Request, status);
                return;
            }

            if (OutputBufferLength == 0) {
                DbgPrint("[pico_driver] ERROR: OutputBufferLength is 0 - no result buffer provided\n");
                status = STATUS_BUFFER_TOO_SMALL;
                WdfRequestComplete(Request, status);
                return;
            }

            // Allocate context for this inference operation
            PINFERENCE_CONTEXT infContext = (PINFERENCE_CONTEXT)ExAllocatePoolWithTag(NonPagedPool, sizeof(INFERENCE_CONTEXT), 'ifnc');
            if (!infContext) {
                DbgPrint("[pico_driver] ERROR: Failed to allocate inference context\n");
                status = STATUS_INSUFFICIENT_RESOURCES;
                WdfRequestComplete(Request, status);
                return;
            }

            // Retrieve input memory (image data to send to device)
            WDFMEMORY inputMemory = NULL;
            status = WdfRequestRetrieveInputMemory(Request, &inputMemory);

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfRequestRetrieveInputMemory failed 0x%x\n", status);
                //ExFreePool(infContext);
                WdfRequestComplete(Request, status);
                return;
            }

            // Retrieve output memory (where to store inference results)
            WDFMEMORY outputMemory = NULL;
            status = WdfRequestRetrieveOutputMemory(Request, &outputMemory);

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
                //ExFreePool(infContext);
                WdfRequestComplete(Request, status);
                return;
            }

            //// Store context information for completion callbacks
            //infContext->OriginalRequest = Request;
            //infContext->OutputMemory = outputMemory;
            //infContext->DeviceContext = pDeviceContext;

            DbgPrint("[pico_driver] Input memory retrieved for inference\n");

            // Format the write request (send image + cmd to device)
            status = WdfUsbTargetPipeFormatRequestForWrite(
                pDeviceContext->WritePipe,
                Request,
                inputMemory,
                NULL
            );

            if (!NT_SUCCESS(status)) {
                DbgPrint("[pico_driver] ERROR: WdfUsbTargetPipeFormatRequestForWrite failed 0x%x\n", status);
                //ExFreePool(infContext);
                WdfRequestComplete(Request, status);
                return;
            }

            DbgPrint("[pico_driver] Request formatted for inference write operation\n");

            // Set completion routine to handle write completion and trigger read
            WdfRequestSetCompletionRoutine(Request, PicoInferenceWriteComplete, NULL);

            // Send the write request asynchronously
            if (!WdfRequestSend(Request, WdfUsbTargetPipeGetIoTarget(pDeviceContext->WritePipe), WDF_NO_SEND_OPTIONS)) {
                status = WdfRequestGetStatus(Request);
                DbgPrint("[pico_driver] ERROR: WdfRequestSend (write) failed 0x%x\n", status);
                //ExFreePool(infContext);
                WdfRequestComplete(Request, status);
                return;
            }

            // Asynchronous - completion will be handled by PicoInferenceWriteComplete callback
            // which will then trigger PicoInferenceReadComplete
            DbgPrint("[pico_driver] Inference write request sent successfully\n");
            return;
        }

        default:
            DbgPrint("[pico_driver] Unknown IOCTL code: 0x%x\n", IoControlCode);
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    WdfRequestComplete(Request, status);
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
    DbgPrint("[pico_driver][%s] Queue 0x%p, Request 0x%p ActionFlags %d\n",
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
