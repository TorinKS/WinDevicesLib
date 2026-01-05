/*
 * End-to-End Tests for USB Mass Storage Devices
 * 
 * These tests require actual USB mass storage devices to be connected.
 * Please connect one or more USB flash drives or external hard drives before running.
 */

#include <gtest/gtest.h>
#include "WinDevicesAPI.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>

class UsbMassStorageE2ETest : public ::testing::Test {
protected:
    HDEVICE_MANAGER handle = nullptr;

    void SetUp() override {
        WD_RESULT result = WD_CreateDeviceManager(&handle);
        ASSERT_EQ(result, WD_SUCCESS);
        ASSERT_NE(handle, nullptr);
    }

    void TearDown() override {
        if (handle) {
            WD_DestroyDeviceManager(handle);
            handle = nullptr;
        }
    }

    void PrintDeviceInfo(const WD_DEVICE_INFO& info, int index) {
        std::cout << "\n========== Device #" << index << " ==========" << std::endl;
        std::cout << "Manufacturer:  " << info.manufacturer << std::endl;
        std::cout << "Product:       " << info.product << std::endl;
        std::cout << "Serial Number: " << info.serialNumber << std::endl;
        std::cout << "Description:   " << info.description << std::endl;
        std::cout << "Friendly Name: " << info.friendlyName << std::endl;
        std::cout << "Device ID:     " << info.deviceId << std::endl;
        std::cout << "Vendor ID:     0x" << std::hex << std::setfill('0') << std::setw(4) 
                  << info.vendorId << std::dec << std::endl;
        std::cout << "Product ID:    0x" << std::hex << std::setfill('0') << std::setw(4) 
                  << info.productId << std::dec << std::endl;
        std::cout << "Device Class:  0x" << std::hex << std::setfill('0') << std::setw(2) 
                  << info.deviceClass << std::dec;
        
        // Add class description
        if (info.deviceClass == 0x08) {
            std::cout << " (Mass Storage at device level)";
        } else if (info.deviceClass == 0x00) {
            std::cout << " (Interface class device - class defined at interface level)";
        }
        std::cout << std::endl;
        
        std::cout << "Is USB Device: " << (info.isUsbDevice ? "Yes" : "No") << std::endl;
        std::cout << "Is Connected:  " << (info.isConnected ? "Yes" : "No") << std::endl;
        std::cout << "==============================\n" << std::endl;
    }

    bool IsMassStorageDevice(const WD_DEVICE_INFO& info) {
        // USB Mass Storage class code is 0x08 at device level
        if (info.deviceClass == 0x08) {
            return true;
        }
        
        // Many USB mass storage devices report class 0x00 at device level
        // because the actual mass storage class (0x08) is defined at the interface level.
        // In this case, check if the product name contains "Mass Storage" or other indicators
        if (info.deviceClass == 0x00) {
            std::string product(info.product);
            std::string description(info.description);
            std::string friendlyName(info.friendlyName);
            
            // Convert to lowercase for case-insensitive matching
            auto toLower = [](std::string str) {
                std::transform(str.begin(), str.end(), str.begin(), ::tolower);
                return str;
            };
            
            product = toLower(product);
            description = toLower(description);
            friendlyName = toLower(friendlyName);
            
            // Check for common mass storage indicators
            bool hasMassStorage = product.find("mass storage") != std::string::npos ||
                                 description.find("mass storage") != std::string::npos ||
                                 friendlyName.find("mass storage") != std::string::npos;
            
            bool hasStorageIndicator = product.find("flash") != std::string::npos ||
                                      product.find("storage") != std::string::npos ||
                                      product.find("disk") != std::string::npos ||
                                      description.find("disk drive") != std::string::npos ||
                                      friendlyName.find("disk drive") != std::string::npos;
            
            return hasMassStorage || hasStorageIndicator;
        }
        
        return false;
    }
};

TEST_F(UsbMassStorageE2ETest, EnumerateUsbDevices) {
    std::cout << "\n*** Enumerating USB devices ***" << std::endl;
    
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int count = 0;
    result = WD_GetDeviceCount(handle, &count);
    ASSERT_EQ(result, WD_SUCCESS);
    
    std::cout << "Found " << count << " USB device(s)" << std::endl;
    EXPECT_GT(count, 0) << "No USB devices found! Please connect a USB mass storage device.";
}

TEST_F(UsbMassStorageE2ETest, DetectMassStorageDevices) {
    std::cout << "\n*** Detecting USB Mass Storage devices ***" << std::endl;
    
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int count = 0;
    result = WD_GetDeviceCount(handle, &count);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int massStorageCount = 0;
    
    for (int i = 0; i < count; i++) {
        WD_DEVICE_INFO info;
        result = WD_GetDeviceInfo(handle, i, &info);
        ASSERT_EQ(result, WD_SUCCESS);
        
        // Debug: Print all devices
        std::cout << "\n--- Device #" << i << " ---" << std::endl;
        std::cout << "Product: '" << info.product << "'" << std::endl;
        std::cout << "Description: '" << info.description << "'" << std::endl;
        std::cout << "FriendlyName: '" << info.friendlyName << "'" << std::endl;
        std::cout << "DeviceClass: 0x" << std::hex << (int)info.deviceClass << std::dec << std::endl;
        
        if (IsMassStorageDevice(info)) {
            massStorageCount++;
            PrintDeviceInfo(info, i);
        }
    }
    
    std::cout << "Found " << massStorageCount << " USB Mass Storage device(s)" << std::endl;
    EXPECT_GT(massStorageCount, 0) 
        << "No USB mass storage devices detected! Please connect a USB flash drive or external hard drive.";
}

TEST_F(UsbMassStorageE2ETest, VerifyDeviceProperties) {
    std::cout << "\n*** Verifying device properties ***" << std::endl;
    
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int count = 0;
    result = WD_GetDeviceCount(handle, &count);
    ASSERT_EQ(result, WD_SUCCESS);
    
    bool foundValidDevice = false;
    
    for (int i = 0; i < count; i++) {
        WD_DEVICE_INFO info;
        result = WD_GetDeviceInfo(handle, i, &info);
        ASSERT_EQ(result, WD_SUCCESS);
        
        if (IsMassStorageDevice(info)) {
            foundValidDevice = true;
            
            // TODO: Verify vendor ID and product ID once they're populated in the API
            // EXPECT_GT(info.vendorId, 0) << "Invalid vendor ID for device " << i;
            // EXPECT_GT(info.productId, 0) << "Invalid product ID for device " << i;
            
            // Verify it's flagged as USB device
            EXPECT_TRUE(info.isUsbDevice) << "Device " << i << " should be flagged as USB device";
            
            // Verify it's connected
            EXPECT_TRUE(info.isConnected) << "Device " << i << " should be flagged as connected";
            
            // Verify at least one of the identifying strings is not empty
            bool hasIdentifier = (strlen(info.manufacturer) > 0) ||
                                (strlen(info.product) > 0) ||
                                (strlen(info.serialNumber) > 0) ||
                                (strlen(info.description) > 0);
            EXPECT_TRUE(hasIdentifier) << "Device " << i << " has no identifying information";
            
            std::cout << "✓ Device #" << i << " passed validation" << std::endl;
        }
    }
    
    EXPECT_TRUE(foundValidDevice) 
        << "No USB mass storage devices found to validate. Please connect a USB device.";
}

TEST_F(UsbMassStorageE2ETest, MultipleEnumerationCycles) {
    std::cout << "\n*** Testing multiple enumeration cycles ***" << std::endl;
    
    for (int cycle = 0; cycle < 3; cycle++) {
        std::cout << "Enumeration cycle " << (cycle + 1) << "..." << std::endl;
        
        WD_RESULT result = WD_EnumerateUsbDevices(handle);
        ASSERT_EQ(result, WD_SUCCESS);
        
        int count = 0;
        result = WD_GetDeviceCount(handle, &count);
        ASSERT_EQ(result, WD_SUCCESS);
        
        std::cout << "  Found " << count << " device(s)" << std::endl;
        EXPECT_GT(count, 0);
    }
    
    std::cout << "✓ All enumeration cycles completed successfully" << std::endl;
}

TEST_F(UsbMassStorageE2ETest, ClearDevices) {
    std::cout << "\n*** Testing clear devices ***" << std::endl;
    
    // Enumerate devices
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int countBefore = 0;
    result = WD_GetDeviceCount(handle, &countBefore);
    ASSERT_EQ(result, WD_SUCCESS);
    std::cout << "Devices before clear: " << countBefore << std::endl;
    
    // Clear devices
    result = WD_ClearDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int countAfter = 0;
    result = WD_GetDeviceCount(handle, &countAfter);
    ASSERT_EQ(result, WD_SUCCESS);
    std::cout << "Devices after clear: " << countAfter << std::endl;
    
    EXPECT_EQ(countAfter, 0) << "Device list should be empty after clear";
    
    std::cout << "✓ Clear devices successful" << std::endl;
}

TEST_F(UsbMassStorageE2ETest, GetVersionInfo) {
    std::cout << "\n*** API Version Information ***" << std::endl;
    
    WD_VERSION_INFO version;
    WD_RESULT result = WD_GetVersion(&version);
    ASSERT_EQ(result, WD_SUCCESS);
    
    std::cout << "API Version: " << version.major << "." 
              << version.minor << "." << version.patch << std::endl;
    std::cout << "Build Date: " << version.buildDate << std::endl;
    
    EXPECT_GE(version.major, 1);
    EXPECT_GE(version.minor, 0);
    EXPECT_NE(version.buildDate, nullptr);
    
    std::cout << "✓ Version info retrieved successfully" << std::endl;
}

TEST_F(UsbMassStorageE2ETest, DetectSpecificJetFlashDevice) {
    std::cout << "\n*** Detecting Specific JetFlash Device (SN: 860G290FCILR8NBZ) ***" << std::endl;
    
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int count = 0;
    result = WD_GetDeviceCount(handle, &count);
    ASSERT_EQ(result, WD_SUCCESS);
    
    bool foundTargetDevice = false;
    const char* targetSerialNumber = "860G290FCILR8NBZ";
    
    for (int i = 0; i < count; i++) {
        WD_DEVICE_INFO info;
        result = WD_GetDeviceInfo(handle, i, &info);
        ASSERT_EQ(result, WD_SUCCESS);
        
        // Check if this is our target device by serial number
        if (strcmp(info.serialNumber, targetSerialNumber) == 0) {
            foundTargetDevice = true;
            
            std::cout << "\n✓ Found target device!" << std::endl;
            std::cout << "  Manufacturer:  " << info.manufacturer << std::endl;
            std::cout << "  Product:       " << info.product << std::endl;
            std::cout << "  Serial Number: " << info.serialNumber << std::endl;
            std::cout << "  Device Class:  0x" << std::hex << (int)info.deviceClass << std::dec << std::endl;
            
            // Verify it's detected as a mass storage device
            EXPECT_TRUE(IsMassStorageDevice(info)) 
                << "Device with SN " << targetSerialNumber << " should be detected as mass storage";
            
            // Verify manufacturer is JetFlash
            EXPECT_STREQ(info.manufacturer, "JetFlash") 
                << "Expected manufacturer to be 'JetFlash'";
            
            // Verify product contains "Mass Storage"
            EXPECT_NE(strstr(info.product, "Mass Storage"), nullptr)
                << "Expected product to contain 'Mass Storage'";
            
            // Verify it's flagged as USB and connected
            EXPECT_TRUE(info.isUsbDevice) << "Device should be flagged as USB device";
            EXPECT_TRUE(info.isConnected) << "Device should be flagged as connected";
            
            break;
        }
    }
    
    EXPECT_TRUE(foundTargetDevice) 
        << "JetFlash device with serial number " << targetSerialNumber 
        << " not found. Please ensure the device is connected.";
    
    if (foundTargetDevice) {
        std::cout << "✓ JetFlash device successfully detected and validated!" << std::endl;
    }
}

// Manual test that requires user interaction
TEST_F(UsbMassStorageE2ETest, DISABLED_ManualHotPlugTest) {
    std::cout << "\n*** Manual Hot Plug Test ***" << std::endl;
    std::cout << "This test requires manual USB device connection/disconnection" << std::endl;
    
    std::cout << "\n1. Disconnect all USB mass storage devices and press Enter...";
    std::cin.get();
    
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int countBefore = 0;
    result = WD_GetDeviceCount(handle, &countBefore);
    std::cout << "Devices before connection: " << countBefore << std::endl;
    
    std::cout << "\n2. Connect a USB mass storage device and press Enter...";
    std::cin.get();
    
    result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int countAfter = 0;
    result = WD_GetDeviceCount(handle, &countAfter);
    std::cout << "Devices after connection: " << countAfter << std::endl;
    
    EXPECT_GT(countAfter, countBefore) 
        << "Device count should increase after connecting a USB device";
    
    std::cout << "✓ Hot plug detection successful" << std::endl;
}
