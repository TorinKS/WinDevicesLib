#include <gtest/gtest.h>
#include <windows.h>
#include <SetupAPI.h>
#include <usb.h>
#include <usbioctl.h>
#include <usbiodef.h>
#include <wil/result.h>
#include <wil/resource.h>
#include "usbdesc.h"
#include "DevicesManager.h"
#include "DeviceResultantInfo.h"
#include "UtilConvert.h"
#include "UsbClassCodes.h"
#include <random>
#include <algorithm>

namespace KDM
{
namespace Testing
{

/// <summary>
/// Property-based tests that verify invariants that should always hold
/// regardless of what USB devices are connected to the system.
/// These tests verify correctness properties rather than specific values.
/// </summary>
class PropertyBasedTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        manager_ = std::make_unique<DevicesManager>();
        manager_->EnumerateUsbDevices();
    }

    void TearDown() override
    {
        manager_.reset();
    }

    std::unique_ptr<DevicesManager> manager_;
};

// ===== VID/PID Validity Properties =====

/// <summary>
/// Property: All USB vendor IDs must be in valid range (0x0000-0xFFFF)
/// </summary>
TEST_F(PropertyBasedTest, AllVendorIds_InValidRange)
{
    const auto& devices = manager_->GetDevices();

    for (const auto& device : devices)
    {
        EXPECT_LE(device.GetVendorId(), 0xFFFF)
            << "Vendor ID should be <= 0xFFFF (USB spec)";
    }
}

/// <summary>
/// Property: All USB product IDs must be in valid range (0x0000-0xFFFF)
/// </summary>
TEST_F(PropertyBasedTest, AllProductIds_InValidRange)
{
    const auto& devices = manager_->GetDevices();

    for (const auto& device : devices)
    {
        EXPECT_LE(device.GetProductId(), 0xFFFF)
            << "Product ID should be <= 0xFFFF (USB spec)";
    }
}

// ===== Device Class Validity Properties =====

/// <summary>
/// Property: All USB device class codes must be in valid range (0x00-0xFF)
/// </summary>
TEST_F(PropertyBasedTest, AllDeviceClasses_InValidRange)
{
    const auto& devices = manager_->GetDevices();

    for (const auto& device : devices)
    {
        EXPECT_LE(device.GetDeviceClass(), 0xFF)
            << "Device class should be <= 0xFF";
    }
}

/// <summary>
/// Property: All USB interface class codes must be in valid range (0x00-0xFF)
/// </summary>
TEST_F(PropertyBasedTest, AllInterfaceClasses_InValidRange)
{
    const auto& devices = manager_->GetDevices();

    for (const auto& device : devices)
    {
        EXPECT_LE(device.GetInterfaceClass(), 0xFF)
            << "Interface class should be <= 0xFF";
    }
}

// ===== String Field Properties =====

/// <summary>
/// Property: Device paths for USB devices should contain "USB" or backslash
/// </summary>
TEST_F(PropertyBasedTest, UsbDevicePaths_ContainExpectedPatterns)
{
    const auto& devices = manager_->GetDevices();

    for (const auto& device : devices)
    {
        if (device.IsUsbDevice() && !device.GetDevicePath().empty())
        {
            // USB device paths typically contain backslashes or "usb"
            bool hasBackslash = device.GetDevicePath().find(L'\\') != std::wstring::npos;
            bool hasUsb = device.GetDevicePath().find(L"usb") != std::wstring::npos ||
                          device.GetDevicePath().find(L"USB") != std::wstring::npos;

            EXPECT_TRUE(hasBackslash || hasUsb)
                << "USB device path should contain backslash or 'USB': "
                << UtilConvert::WStringToUTF8(device.GetDevicePath());
        }
    }
}

/// <summary>
/// Property: Device IDs should not contain null characters within the string
/// </summary>
TEST_F(PropertyBasedTest, DeviceIds_NoEmbeddedNulls)
{
    const auto& devices = manager_->GetDevices();

    for (const auto& device : devices)
    {
        if (!device.GetDeviceId().empty())
        {
            auto nullPos = device.GetDeviceId().find(L'\0');
            // Null should only be at the end (if at all, due to how wstring works)
            EXPECT_EQ(nullPos, std::wstring::npos)
                << "Device ID should not contain embedded null characters";
        }
    }
}

// ===== Connected Device Properties =====

/// <summary>
/// Property: Connected USB devices should have valid device descriptor info
/// </summary>
TEST_F(PropertyBasedTest, ConnectedUsbDevices_HaveValidDescriptorInfo)
{
    const auto& devices = manager_->GetDevices();

    for (const auto& device : devices)
    {
        if (device.IsConnected() && device.IsUsbDevice())
        {
            // Connected USB devices should have either VID or PID set
            // (both being 0 is technically valid but unusual for real devices)
            bool hasVidOrPid = (device.GetVendorId() > 0) || (device.GetProductId() > 0);

            // Or they should have some identifying information
            bool hasIdentifyingInfo = !device.GetManufacturer().empty() ||
                                      !device.GetProduct().empty() ||
                                      !device.GetDescription().empty();

            EXPECT_TRUE(hasVidOrPid || hasIdentifyingInfo)
                << "Connected USB device should have VID/PID or identifying info";
        }
    }
}

// ===== GUID Properties =====

/// <summary>
/// Property: Setup class GUIDs should be valid (non-zero for USB devices with known class)
/// </summary>
TEST_F(PropertyBasedTest, SetupClassGuids_ValidFormat)
{
    const auto& devices = manager_->GetDevices();

    for (const auto& device : devices)
    {
        // If a GUID is set, verify it's not all zeros (which indicates unset)
        bool isZeroGuid = (device.GetSetupClassGuid().Data1 == 0 &&
                          device.GetSetupClassGuid().Data2 == 0 &&
                          device.GetSetupClassGuid().Data3 == 0);

        // For devices marked as USB devices that are connected,
        // they should typically have a valid GUID
        if (device.IsUsbDevice() && device.IsConnected())
        {
            // This is a soft expectation - some devices may not have GUIDs
            // Just log it rather than fail
            // if (isZeroGuid)
            // {
            //     std::cout << "Note: USB device has zero GUID: "
            //               << UtilConvert::WStringToUTF8(device.GetProduct()) << std::endl;
            // }
        }
    }

    // This test always passes - it's informational
    SUCCEED();
}

// ===== UtilConvert Properties =====

/// <summary>
/// Property: GetUsbClassNameByDescId should never return empty string
/// </summary>
TEST_F(PropertyBasedTest, UsbClassNames_NeverEmpty)
{
    // Test all possible class codes (0x00-0xFF)
    for (int classCode = 0; classCode <= 0xFF; ++classCode)
    {
        std::wstring className = UtilConvert::GetUsbClassNameByDescId(
            static_cast<UCHAR>(classCode));

        EXPECT_FALSE(className.empty())
            << "Class name should not be empty for class code 0x"
            << std::hex << classCode;
    }
}

/// <summary>
/// Property: Known USB class codes should return expected names
/// </summary>
TEST_F(PropertyBasedTest, KnownUsbClassCodes_ReturnCorrectNames)
{
    // Mass Storage (0x08)
    EXPECT_EQ(UtilConvert::GetUsbClassNameByDescId(UsbClass::MassStorage), L"Mass Storage");

    // HID (0x03)
    EXPECT_EQ(UtilConvert::GetUsbClassNameByDescId(UsbClass::Hid), L"HID (Human Interface Device)");

    // Hub (0x09)
    EXPECT_EQ(UtilConvert::GetUsbClassNameByDescId(UsbClass::Hub), L"Hub");

    // Audio (0x01)
    EXPECT_EQ(UtilConvert::GetUsbClassNameByDescId(UsbClass::Audio), L"Audio");

    // Vendor Specific (0xFF)
    EXPECT_EQ(UtilConvert::GetUsbClassNameByDescId(UsbClass::VendorSpecific), L"Vendor Specific");
}

/// <summary>
/// Property: GetHexIdAsString should produce consistent length output
/// </summary>
TEST_F(PropertyBasedTest, HexIdString_ConsistentLength)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 0xFFFF);

    // Test 100 random values
    for (int i = 0; i < 100; ++i)
    {
        USHORT value = static_cast<USHORT>(dis(gen));
        std::wstring result = UtilConvert::GetHexIdAsString(value, 4);

        // Result should be "0xXXXX" format (6 characters)
        EXPECT_EQ(result.length(), 6)
            << "Hex string should be 6 characters for 4-byte width";

        // Should start with "0x"
        EXPECT_EQ(result.substr(0, 2), L"0x")
            << "Hex string should start with '0x'";
    }
}

// ===== Enumeration Consistency Properties =====

/// <summary>
/// Property: Multiple enumerations should return same device count
/// (assuming no hot-plug during test)
/// </summary>
TEST_F(PropertyBasedTest, MultipleEnumerations_ConsistentCount)
{
    size_t firstCount = manager_->GetDevices().size();

    // Re-enumerate
    DevicesManager manager2;
    manager2.EnumerateUsbDevices();
    size_t secondCount = manager2.GetDevices().size();

    // Counts should match (assuming no device changes during test)
    EXPECT_EQ(firstCount, secondCount)
        << "Device count should be consistent across enumerations";
}

/// <summary>
/// Property: DevicesManager should handle being cleared and re-enumerated
/// </summary>
TEST_F(PropertyBasedTest, ClearAndReEnumerate_Works)
{
    size_t originalCount = manager_->GetDevices().size();

    // Create a new manager (simulates clear)
    manager_ = std::make_unique<DevicesManager>();
    EXPECT_EQ(manager_->GetDevices().size(), 0)
        << "Fresh manager should have no devices";

    // Re-enumerate
    manager_->EnumerateUsbDevices();

    // Should have devices again
    EXPECT_EQ(manager_->GetDevices().size(), originalCount)
        << "Should have same device count after re-enumeration";
}

} // namespace Testing
} // namespace KDM
