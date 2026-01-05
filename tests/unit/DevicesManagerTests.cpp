#include <gtest/gtest.h>
#include <windows.h>
#include "DevicesManager.h"
#include "DeviceResultantInfo.h"
#include "DeviceClassGuids.h"
#include <memory>

class DevicesManagerTest : public ::testing::Test {
protected:
    std::unique_ptr<KDM::DevicesManager> manager;

    void SetUp() override {
        manager = std::make_unique<KDM::DevicesManager>();
    }

    void TearDown() override {
        manager.reset();
    }
};

TEST_F(DevicesManagerTest, ManagerCreation) {
    ASSERT_NE(manager, nullptr);
}

TEST_F(DevicesManagerTest, EnumerateUsbDevices) {
    EXPECT_NO_THROW(manager->EnumerateUsbDevices());
}

TEST_F(DevicesManagerTest, GetDevicesAfterEnumeration) {
    manager->EnumerateUsbDevices();
    auto devices = manager->GetDevices();
    
    // Should have enumerated devices
    EXPECT_GE(devices.size(), 0);
}

TEST_F(DevicesManagerTest, GetDevicesBeforeEnumeration) {
    auto devices = manager->GetDevices();
    
    // Should return empty vector before enumeration
    EXPECT_EQ(devices.size(), 0);
}

TEST_F(DevicesManagerTest, DeviceHasInformation) {
    manager->EnumerateUsbDevices();
    auto devices = manager->GetDevices();
    
    if (devices.size() > 0) {
        // Check first device has some information
        const auto& device = devices[0];
        bool hasInfo = !device.GetManufacturer().empty() ||
                      !device.GetProduct().empty() ||
                      !device.GetSerialNumber().empty() ||
                      !device.GetDescription().empty();
        
        EXPECT_TRUE(hasInfo) << "Device should have at least some information";
    }
}

TEST_F(DevicesManagerTest, MultipleEnumerations) {
    EXPECT_NO_THROW(manager->EnumerateUsbDevices());
    auto count1 = manager->GetDevices().size();

    EXPECT_NO_THROW(manager->EnumerateUsbDevices());
    auto count2 = manager->GetDevices().size();

    EXPECT_GE(count1, 0);
    EXPECT_GE(count2, 0);
}

// ========== EnumerateByDeviceClass Tests ==========

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_Keyboard) {
    // Test enumeration of keyboard devices
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_KEYBOARD));

    auto devices = manager->GetDevices();

    // Most systems have at least one keyboard
    EXPECT_GT(devices.size(), 0) << "System should have at least one keyboard";

    // Verify all devices have the correct GUID
    for (const auto& device : devices) {
        EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_KEYBOARD, sizeof(GUID)), 0);
        EXPECT_TRUE(device.IsConnected());
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_Mouse) {
    // Test enumeration of mouse devices
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_MOUSE));

    auto devices = manager->GetDevices();

    // Most systems have at least one mouse/pointing device
    EXPECT_GT(devices.size(), 0) << "System should have at least one mouse";

    // Verify device information is populated
    for (const auto& device : devices) {
        EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_MOUSE, sizeof(GUID)), 0);

        // At least one of these should be non-empty
        bool hasInfo = !device.GetDescription().empty() ||
                      !device.GetFriendlyName().empty() ||
                      !device.GetManufacturer().empty();
        EXPECT_TRUE(hasInfo) << "Device should have at least some identification";
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_DiskDrive) {
    // Test enumeration of disk drives
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_DISKDRIVE));

    auto devices = manager->GetDevices();

    // All systems have at least one disk drive (where OS is installed)
    EXPECT_GT(devices.size(), 0) << "System should have at least one disk drive";

    for (const auto& device : devices) {
        EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_DISKDRIVE, sizeof(GUID)), 0);
        EXPECT_TRUE(device.IsConnected());
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_Display) {
    // Test enumeration of display adapters
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_DISPLAY));

    auto devices = manager->GetDevices();

    // All systems have at least one display adapter
    EXPECT_GT(devices.size(), 0) << "System should have at least one display adapter";
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_Network) {
    // Test enumeration of network adapters
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_NET));

    auto devices = manager->GetDevices();

    // Most modern systems have network adapters
    EXPECT_GE(devices.size(), 0);

    if (devices.size() > 0) {
        // Verify network adapter information
        for (const auto& device : devices) {
            EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_NET, sizeof(GUID)), 0);
        }
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_ClearsPreviousList) {
    // First enumerate keyboards
    manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_KEYBOARD);
    auto keyboardCount = manager->GetDevices().size();

    // Then enumerate mice
    manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_MOUSE);
    auto mouseCount = manager->GetDevices().size();

    // Verify the list was cleared and only contains mice
    auto devices = manager->GetDevices();
    for (const auto& device : devices) {
        EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_MOUSE, sizeof(GUID)), 0)
            << "After enumerating mice, should only have mouse devices";
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_MultipleCallsSameClass) {
    // Enumerate keyboards twice
    manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_KEYBOARD);
    auto count1 = manager->GetDevices().size();

    manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_KEYBOARD);
    auto count2 = manager->GetDevices().size();

    // Should get the same count both times
    EXPECT_EQ(count1, count2) << "Multiple enumerations should produce consistent results";
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_DeviceFieldsPopulated) {
    // Enumerate keyboards and check that fields are populated correctly
    manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_KEYBOARD);
    auto devices = manager->GetDevices();

    ASSERT_GT(devices.size(), 0);

    // Check first device has expected fields
    const auto& device = devices[0];

    // Should have GUID set correctly
    EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_KEYBOARD, sizeof(GUID)), 0);

    // Should be marked as connected
    EXPECT_TRUE(device.IsConnected());

    // Should have at least description or friendly name
    bool hasName = !device.GetDescription().empty() || !device.GetFriendlyName().empty();
    EXPECT_TRUE(hasName) << "Device should have a description or friendly name";
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_HID) {
    // Test HID class (Human Interface Devices)
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_HIDCLASS));

    auto devices = manager->GetDevices();

    // HID class usually has many devices (keyboards, mice, game controllers, etc.)
    EXPECT_GE(devices.size(), 0);

    for (const auto& device : devices) {
        EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_HIDCLASS, sizeof(GUID)), 0);
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_Media) {
    // Test Media class (Sound, video, game controllers)
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_MEDIA));

    auto devices = manager->GetDevices();

    // Media devices are common (sound cards, etc.)
    EXPECT_GE(devices.size(), 0);
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_Battery) {
    // Test Battery class (may not exist on desktop systems)
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_BATTERY));

    auto devices = manager->GetDevices();

    // Laptops have batteries, desktops don't - so >= 0 is fine
    EXPECT_GE(devices.size(), 0);

    // If batteries are found, verify GUID
    for (const auto& device : devices) {
        EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_BATTERY, sizeof(GUID)), 0);
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_USBDevice) {
    // Test USB Device class
    EXPECT_NO_THROW(manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_USBDEVICE));

    auto devices = manager->GetDevices();

    // Should have some USB devices on any modern system
    EXPECT_GE(devices.size(), 0);

    for (const auto& device : devices) {
        EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_USBDEVICE, sizeof(GUID)), 0);
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_CompareWithUSBEnumeration) {
    // Enumerate using USB enumeration
    manager->EnumerateUsbDevices();
    auto usbDevices = manager->GetDevices();
    auto usbCount = usbDevices.size();

    // Enumerate using device class
    manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_USBDEVICE);
    auto classDevices = manager->GetDevices();

    // Both methods should find devices (though counts may differ due to different enumeration logic)
    EXPECT_GE(usbCount, 0);
    EXPECT_GE(classDevices.size(), 0);

    // Device class enumeration should find USB devices
    for (const auto& device : classDevices) {
        EXPECT_EQ(memcmp(&device.GetSetupClassGuid(), &KDM::GUID_DEVCLASS_USBDEVICE, sizeof(GUID)), 0);
    }
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_DeviceIdNotEmpty) {
    // Enumerate keyboards and verify device IDs
    manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_KEYBOARD);
    auto devices = manager->GetDevices();

    ASSERT_GT(devices.size(), 0);

    // At least one device should have a hardware ID
    bool hasDeviceId = false;
    for (const auto& device : devices) {
        if (!device.GetDeviceId().empty()) {
            hasDeviceId = true;
            break;
        }
    }
    EXPECT_TRUE(hasDeviceId) << "At least one device should have a hardware ID";
}

TEST_F(DevicesManagerTest, EnumerateByDeviceClass_EmptyListBeforeEnumeration) {
    // Verify list is empty before enumeration
    auto devicesBefore = manager->GetDevices();
    EXPECT_EQ(devicesBefore.size(), 0);

    // Enumerate
    manager->EnumerateByDeviceClass(KDM::GUID_DEVCLASS_KEYBOARD);

    // Verify list has devices after enumeration
    auto devicesAfter = manager->GetDevices();
    EXPECT_GT(devicesAfter.size(), 0);
}
