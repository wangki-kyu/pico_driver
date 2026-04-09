#include <iostream>
#include <windows.h>

#include "../include/util.hpp"
//#include "../include/Public.h"

#pragma comment(lib, "user32.lib")

int main()
{
    std::cout << "Pico Driver Test Application" << std::endl;
    std::cout << "=============================" << std::endl;

    // Find and open the pico driver device
    HANDLE hDevice = FindPicoDriverDevice();

    if (hDevice != INVALID_HANDLE_VALUE) {
        std::cout << "\nDriver handle obtained successfully!" << std::endl;

        // LED Control Examples
        DWORD bytesReturned = 0;
        UCHAR command = 0;
        UCHAR status = 0;

        // Turn LED ON
        std::cout << "\nTurning LED ON..." << std::endl;
        command = LED_ON;
        if (DeviceIoControl(hDevice, IOCTL_PICO_TEST_WRITE, &command, sizeof(command), NULL, 0, &bytesReturned, NULL)) {
            std::cout << "LED ON - Success!" << std::endl;
        } else {
            std::cerr << "LED ON - Failed: " << GetLastError() << std::endl;
        }

        Sleep(1500);

        // Turn LED OFF
        std::cout << "\nTurning LED OFF..." << std::endl;
        command = LED_OFF;
        if (DeviceIoControl(hDevice, IOCTL_PICO_TEST_WRITE, &command, sizeof(command), NULL, 0, &bytesReturned, NULL)) {
            std::cout << "LED OFF - Success!" << std::endl;
        } else {
            std::cerr << "LED OFF - Failed: " << GetLastError() << std::endl;
        }

        //// Toggle LED
        //std::cout << "\nToggling LED..." << std::endl;
        //command = LED_TOGGLE;
        //if (DeviceIoControl(hDevice, IOCTL_PICO_TEST_WRITE, &command, sizeof(command), NULL, 0, &bytesReturned, NULL)) {
        //    std::cout << "LED TOGGLE - Success!" << std::endl;
        //} else {
        //    std::cerr << "LED TOGGLE - Failed: " << GetLastError() << std::endl;
        //}

        //// Get LED Status
        //std::cout << "\nGetting LED Status..." << std::endl;
        //command = LED_STATUS;
        //if (DeviceIoControl(hDevice, IOCTL_PICO_TEST_READ, &command, sizeof(command), &status, sizeof(status), &bytesReturned, NULL)) {
        //    std::cout << "LED Status: " << (int)status << " (0=OFF, 1=ON)" << std::endl;
        //} else {
        //    std::cerr << "LED STATUS - Failed: " << GetLastError() << std::endl;
        //}

        CloseHandle(hDevice);
    } else {
        std::cerr << "\nFailed to find or open the pico driver device." << std::endl;
        return 1;
    }

    MessageBox(NULL, L"test", L"test", 0);

    return 0;
}
