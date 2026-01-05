/*
 * End-to-End Tests for General Device Enumeration
 * Tests the C API with real hardware
 */

#include <gtest/gtest.h>
#include "WinDevicesAPI.h"
#include <iostream>

class DeviceEnumerationE2ETest : public ::testing::Test {
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
};

TEST_F(DeviceEnumerationE2ETest, CreateAndDestroyManager) {
    // Manager is created in SetUp and destroyed in TearDown
    SUCCEED();
}

TEST_F(DeviceEnumerationE2ETest, EnumerateAllDevices) {
    std::cout << "\n*** Enumerating all devices ***" << std::endl;
    
    WD_RESULT result = WD_EnumerateAllDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int count = 0;
    result = WD_GetDeviceCount(handle, &count);
    ASSERT_EQ(result, WD_SUCCESS);
    
    std::cout << "Found " << count << " total device(s)" << std::endl;
    EXPECT_GT(count, 0) << "Should find at least some devices on the system";
}

TEST_F(DeviceEnumerationE2ETest, CompareUsbVsAllDevices) {
    std::cout << "\n*** Comparing USB vs All device enumeration ***" << std::endl;
    
    // Enumerate USB devices
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int usbCount = 0;
    result = WD_GetDeviceCount(handle, &usbCount);
    ASSERT_EQ(result, WD_SUCCESS);
    std::cout << "USB devices: " << usbCount << std::endl;
    
    // Clear and enumerate all devices
    result = WD_ClearDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    result = WD_EnumerateAllDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int allCount = 0;
    result = WD_GetDeviceCount(handle, &allCount);
    ASSERT_EQ(result, WD_SUCCESS);
    std::cout << "All devices: " << allCount << std::endl;
    
    EXPECT_GE(allCount, usbCount) 
        << "All devices count should be >= USB devices count";
}

TEST_F(DeviceEnumerationE2ETest, InvalidHandleReturnsError) {
    WD_RESULT result = WD_EnumerateUsbDevices(nullptr);
    EXPECT_EQ(result, WD_ERROR_INVALID_HANDLE);
    
    result = WD_GetDeviceCount(nullptr, nullptr);
    EXPECT_EQ(result, WD_ERROR_INVALID_HANDLE);
}

TEST_F(DeviceEnumerationE2ETest, NullPointerReturnsError) {
    WD_RESULT result = WD_GetDeviceCount(handle, nullptr);
    EXPECT_EQ(result, WD_ERROR_NULL_POINTER);
    
    result = WD_GetDeviceInfo(handle, 0, nullptr);
    EXPECT_EQ(result, WD_ERROR_NULL_POINTER);
}

TEST_F(DeviceEnumerationE2ETest, InvalidIndexReturnsError) {
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    WD_DEVICE_INFO info;
    result = WD_GetDeviceInfo(handle, -1, &info);
    EXPECT_EQ(result, WD_ERROR_INVALID_INDEX);
    
    result = WD_GetDeviceInfo(handle, 999999, &info);
    EXPECT_EQ(result, WD_ERROR_INVALID_INDEX);
}

TEST_F(DeviceEnumerationE2ETest, ErrorMessages) {
    std::cout << "\n*** Testing error messages ***" << std::endl;
    
    const char* msg;
    
    msg = WD_GetErrorMessage(WD_SUCCESS);
    EXPECT_NE(msg, nullptr);
    std::cout << "WD_SUCCESS: " << msg << std::endl;
    
    msg = WD_GetErrorMessage(WD_ERROR_INVALID_HANDLE);
    EXPECT_NE(msg, nullptr);
    std::cout << "WD_ERROR_INVALID_HANDLE: " << msg << std::endl;
    
    msg = WD_GetErrorMessage(WD_ERROR_NULL_POINTER);
    EXPECT_NE(msg, nullptr);
    std::cout << "WD_ERROR_NULL_POINTER: " << msg << std::endl;
    
    msg = WD_GetErrorMessage(static_cast<WD_RESULT>(-999));
    EXPECT_NE(msg, nullptr);
    std::cout << "Unknown error: " << msg << std::endl;
}

TEST_F(DeviceEnumerationE2ETest, IterateAllDevices) {
    std::cout << "\n*** Iterating through all enumerated devices ***" << std::endl;
    
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int count = 0;
    result = WD_GetDeviceCount(handle, &count);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int successCount = 0;
    for (int i = 0; i < count; i++) {
        WD_DEVICE_INFO info;
        result = WD_GetDeviceInfo(handle, i, &info);
        if (result == WD_SUCCESS) {
            successCount++;
            
            // Basic validation - at least one field should be populated
            bool hasData = (strlen(info.manufacturer) > 0) ||
                          (strlen(info.product) > 0) ||
                          (strlen(info.description) > 0) ||
                          (info.vendorId > 0);
            
            if (!hasData) {
                std::cout << "Warning: Device #" << i << " has no data" << std::endl;
            }
        }
    }
    
    std::cout << "Successfully retrieved info for " << successCount 
              << " out of " << count << " devices" << std::endl;
    
    EXPECT_EQ(successCount, count) 
        << "Should be able to retrieve info for all enumerated devices";
}

TEST_F(DeviceEnumerationE2ETest, MultipleManagers) {
    std::cout << "\n*** Testing multiple device managers ***" << std::endl;
    
    HDEVICE_MANAGER handle2 = nullptr;
    WD_RESULT result = WD_CreateDeviceManager(&handle2);
    ASSERT_EQ(result, WD_SUCCESS);
    ASSERT_NE(handle2, nullptr);
    
    // Enumerate on first manager
    result = WD_EnumerateUsbDevices(handle);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int count1 = 0;
    result = WD_GetDeviceCount(handle, &count1);
    ASSERT_EQ(result, WD_SUCCESS);
    
    // Enumerate on second manager
    result = WD_EnumerateUsbDevices(handle2);
    ASSERT_EQ(result, WD_SUCCESS);
    
    int count2 = 0;
    result = WD_GetDeviceCount(handle2, &count2);
    ASSERT_EQ(result, WD_SUCCESS);
    
    std::cout << "Manager 1: " << count1 << " devices" << std::endl;
    std::cout << "Manager 2: " << count2 << " devices" << std::endl;
    
    EXPECT_EQ(count1, count2) 
        << "Both managers should enumerate the same number of devices";
    
    // Clean up second manager
    result = WD_DestroyDeviceManager(handle2);
    EXPECT_EQ(result, WD_SUCCESS);
    
    std::cout << "✓ Multiple managers work independently" << std::endl;
}

TEST_F(DeviceEnumerationE2ETest, DISABLED_StressTestEnumeration) {
    std::cout << "\n*** Stress testing enumeration (100 iterations) ***" << std::endl;
    
    const int iterations = 100;
    int failures = 0;
    
    for (int i = 0; i < iterations; i++) {
        WD_RESULT result = WD_EnumerateUsbDevices(handle);
        if (result != WD_SUCCESS) {
            failures++;
        }
        
        if ((i + 1) % 20 == 0) {
            std::cout << "Completed " << (i + 1) << " iterations..." << std::endl;
        }
    }
    
    std::cout << "Completed " << iterations << " iterations with " 
              << failures << " failures" << std::endl;
    
    EXPECT_EQ(failures, 0) << "Should have zero failures in stress test";
    
    // Verify we can still enumerate after stress test
    WD_RESULT result = WD_EnumerateUsbDevices(handle);
    EXPECT_EQ(result, WD_SUCCESS);
    
    int count = 0;
    result = WD_GetDeviceCount(handle, &count);
    EXPECT_EQ(result, WD_SUCCESS);
    EXPECT_GT(count, 0);
    
    std::cout << "✓ Stress test completed successfully" << std::endl;
}
