#include <iostream>
#include <windows.h>

#include "../include/util.hpp"

#pragma comment(lib, "user32.lib")

int main()
{
    std::cout << "Pico Driver Test Application" << std::endl;
    std::cout << "=============================" << std::endl;

    // Find and open the pico driver device
    HANDLE hDevice = FindPicoDriverDevice();

    if (hDevice != INVALID_HANDLE_VALUE) {
        std::cout << "\nDriver handle obtained successfully!" << std::endl;
        // TODO: Add IOCTL calls here

        CloseHandle(hDevice);
    } else {
        std::cerr << "\nFailed to find or open the pico driver device." << std::endl;
        return 1;
    }

    MessageBox(NULL, L"test", L"test", 0);

    return 0;
}
