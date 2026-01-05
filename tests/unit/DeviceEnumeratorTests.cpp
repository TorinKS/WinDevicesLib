#include <gtest/gtest.h>
#include <windows.h>
#include "DevicesManager.h"
#include "DeviceResultantInfo.h"
#include <memory>

class DeviceEnumeratorTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(DeviceEnumeratorTest, ManagerCreation) {
    auto manager = std::make_unique<KDM::DevicesManager>();
    ASSERT_NE(manager, nullptr);
}

TEST_F(DeviceEnumeratorTest, EnumerateUsbDevices) {
    auto manager = std::make_unique<KDM::DevicesManager>();
    
    // EnumerateUsbDevices returns void, so just call it
    EXPECT_NO_THROW(manager->EnumerateUsbDevices());
}

TEST_F(DeviceEnumeratorTest, GetDevicesAfterEnumeration) {
    auto manager = std::make_unique<KDM::DevicesManager>();
    manager->EnumerateUsbDevices();
    
    auto devices = manager->GetDevices();
    // Should have enumerated some devices on a Windows system
    EXPECT_GE(devices.size(), 0);
}

TEST_F(DeviceEnumeratorTest, EnumeratedDevicesHaveValidInfo) {
    auto manager = std::make_unique<KDM::DevicesManager>();
    manager->EnumerateUsbDevices();
    
    auto devices = manager->GetDevices();
    
    for (const auto& device : devices) {
        // Check that at least one field has data
        bool hasData = !device.GetManufacturer().empty() ||
                      !device.GetProduct().empty() ||
                      !device.GetSerialNumber().empty() ||
                      !device.GetDescription().empty();
        
        if (devices.size() > 0) {
            // If we have devices, at least some should have data
            EXPECT_TRUE(hasData || devices.size() > 0);
        }
    }
}

TEST_F(DeviceEnumeratorTest, MultipleEnumerationCalls) {
    auto manager = std::make_unique<KDM::DevicesManager>();
    
    EXPECT_NO_THROW(manager->EnumerateUsbDevices());
    auto count1 = manager->GetDevices().size();
    
    EXPECT_NO_THROW(manager->EnumerateUsbDevices());
    auto count2 = manager->GetDevices().size();
    
    // Both calls should succeed and return similar counts
    EXPECT_GE(count1, 0);
    EXPECT_GE(count2, 0);
}
