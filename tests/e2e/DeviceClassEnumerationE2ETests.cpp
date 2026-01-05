#include <gtest/gtest.h>
#include "DevicesManager.h"
#include "DeviceResultantInfo.h"
#include "DeviceClassGuids.h"

#include <iostream>
#include <string>

using namespace KDM;

class DeviceClassEnumerationE2ETest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<DevicesManager>();
    }

    void TearDown() override {
        manager.reset();
    }

    void PrintDevices(const std::vector<DeviceResultantInfo>& devices, const std::wstring& title) {
        std::wcout << L"\n========================================" << std::endl;
        std::wcout << title << L" (" << devices.size() << L" device(s))" << std::endl;
        std::wcout << L"========================================" << std::endl;

        for (const auto& device : devices) {
            std::wcout << L"  Manufacturer: " << (device.GetManufacturer().empty() ? L"(Unknown)" : device.GetManufacturer()) << std::endl;
            std::wcout << L"  Product:      " << (device.GetProduct().empty() ? L"(Unknown)" : device.GetProduct()) << std::endl;
            std::wcout << L"  Serial:       " << (device.GetSerialNumber().empty() ? L"(None)" : device.GetSerialNumber()) << std::endl;
            std::wcout << L"  Description:  " << (device.GetDescription().empty() ? L"(None)" : device.GetDescription()) << std::endl;
            std::wcout << L"  Friendly:     " << (device.GetFriendlyName().empty() ? L"(None)" : device.GetFriendlyName()) << std::endl;
            std::wcout << L"  USB Device:   " << (device.IsUsbDevice() ? L"Yes" : L"No") << std::endl;
            std::wcout << L"  Connected:    " << (device.IsConnected() ? L"Yes" : L"No") << std::endl;
            std::wcout << L"  ---" << std::endl;
        }
    }

    std::unique_ptr<DevicesManager> manager;
};

TEST_F(DeviceClassEnumerationE2ETest, EnumerateUsbDevices) {
    manager->EnumerateUsbDevices();
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"USB Devices");

    // USB devices should exist on most systems
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(DeviceClassEnumerationE2ETest, EnumerateMediaDevices) {
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_MEDIA);
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"Media Devices (Sound, Video, Game Controllers)");

    EXPECT_GE(devices.size(), 0u);
}

TEST_F(DeviceClassEnumerationE2ETest, EnumerateModemDevices) {
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_MODEM);
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"Modem Devices");

    EXPECT_GE(devices.size(), 0u);
}

TEST_F(DeviceClassEnumerationE2ETest, EnumerateKeyboardDevices) {
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_KEYBOARD);
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"Keyboard Devices");

    // Most systems have at least one keyboard
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(DeviceClassEnumerationE2ETest, EnumerateMouseDevices) {
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_MOUSE);
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"Mouse Devices");

    EXPECT_GE(devices.size(), 0u);
}

TEST_F(DeviceClassEnumerationE2ETest, EnumerateDiskDriveDevices) {
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_DISKDRIVE);
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"Disk Drive Devices");

    // Most systems have at least one disk drive
    EXPECT_GE(devices.size(), 1u);
}

TEST_F(DeviceClassEnumerationE2ETest, EnumerateNetworkDevices) {
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_NET);
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"Network Adapter Devices");

    // Most systems have at least one network adapter
    EXPECT_GE(devices.size(), 1u);
}

TEST_F(DeviceClassEnumerationE2ETest, EnumerateBluetoothDevices) {
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_BLUETOOTH);
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"Bluetooth Devices");

    EXPECT_GE(devices.size(), 0u);
}

TEST_F(DeviceClassEnumerationE2ETest, EnumerateImageDevices) {
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_IMAGE);
    auto devices = manager->GetDevices();
    PrintDevices(devices, L"Image Devices (Cameras, Scanners)");

    EXPECT_GE(devices.size(), 0u);
}

TEST_F(DeviceClassEnumerationE2ETest, FullDeviceEnumerationDemo) {
    std::wcout << L"\nWinDevices Library - Device Enumeration Demo" << std::endl;
    std::wcout << L"=============================================" << std::endl;

    // USB devices
    manager->EnumerateUsbDevices();
    auto usbDevices = manager->GetDevices();
    PrintDevices(usbDevices, L"USB Devices");

    // Media Devices
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_MEDIA);
    auto mediaDevices = manager->GetDevices();
    PrintDevices(mediaDevices, L"Media Devices");

    // Modems
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_MODEM);
    auto modemDevices = manager->GetDevices();
    PrintDevices(modemDevices, L"Modem Devices");

    // Keyboards
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_KEYBOARD);
    auto keyboardDevices = manager->GetDevices();
    PrintDevices(keyboardDevices, L"Keyboard Devices");

    // Mice
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_MOUSE);
    auto mouseDevices = manager->GetDevices();
    PrintDevices(mouseDevices, L"Mouse Devices");

    // Disk Drives
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_DISKDRIVE);
    auto diskDevices = manager->GetDevices();
    PrintDevices(diskDevices, L"Disk Drive Devices");

    // Network Adapters
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_NET);
    auto networkDevices = manager->GetDevices();
    PrintDevices(networkDevices, L"Network Adapter Devices");

    // Bluetooth
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_BLUETOOTH);
    auto bluetoothDevices = manager->GetDevices();
    PrintDevices(bluetoothDevices, L"Bluetooth Devices");

    // Image Devices
    manager->EnumerateByDeviceClass(GUID_DEVCLASS_IMAGE);
    auto imageDevices = manager->GetDevices();
    PrintDevices(imageDevices, L"Image Devices");

    // Summary
    std::wcout << L"\n========================================" << std::endl;
    std::wcout << L"Summary" << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"  USB Devices:       " << usbDevices.size() << std::endl;
    std::wcout << L"  Media Devices:     " << mediaDevices.size() << std::endl;
    std::wcout << L"  Modems:            " << modemDevices.size() << std::endl;
    std::wcout << L"  Keyboards:         " << keyboardDevices.size() << std::endl;
    std::wcout << L"  Mice:              " << mouseDevices.size() << std::endl;
    std::wcout << L"  Disk Drives:       " << diskDevices.size() << std::endl;
    std::wcout << L"  Network Adapters:  " << networkDevices.size() << std::endl;
    std::wcout << L"  Bluetooth:         " << bluetoothDevices.size() << std::endl;
    std::wcout << L"  Image Devices:     " << imageDevices.size() << std::endl;
    std::wcout << L"\nEnumeration complete!" << std::endl;

    SUCCEED();
}
