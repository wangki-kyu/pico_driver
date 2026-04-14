/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that app can find the device and talk to it.
//

#ifdef INITGUID
DEFINE_GUID (GUID_DEVINTERFACE_picodriver,
    0x5202bf06,0x5a10,0x4c9f,0xa3,0xd0,0x7d,0x6b,0xfb,0xc3,0xd6,0x29);
#else
EXTERN_C const GUID GUID_DEVINTERFACE_picodriver;
#endif
// {5202bf06-5a10-4c9f-a3d0-7d6bfbc3d629}

//
// IOCTL definitions for testing
//
#define IOCTL_PICO_TEST_WRITE   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_PICO_TEST_READ    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_PICO_READ_TEMP    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_PICO_DMA_WRITE    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_IN_DIRECT, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_PICO_MODEL_LOAD    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_IN_DIRECT, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_PICO_RUN_INFERENCE    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_IN_DIRECT, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_PICO_READ_INTERRUPT   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

//
// LED Control Commands
//
#define LED_OFF     0x00
#define LED_ON      0x01
#define LED_TOGGLE  0x02
#define LED_STATUS  0x03

//
// Sensor Command Codes
//
#define SENSOR_READ_TEMP  0x10
