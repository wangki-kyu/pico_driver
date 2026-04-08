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

DEFINE_GUID (GUID_DEVINTERFACE_picodriver,
    0x5202bf06,0x5a10,0x4c9f,0xa3,0xd0,0x7d,0x6b,0xfb,0xc3,0xd6,0x29);
// {5202bf06-5a10-4c9f-a3d0-7d6bfbc3d629}

//
// IOCTL definitions for testing
//
#define IOCTL_PICO_TEST_WRITE   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
#define IOCTL_PICO_TEST_READ    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)
