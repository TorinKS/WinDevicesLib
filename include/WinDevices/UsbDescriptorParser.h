#pragma once

#include <Windows.h>
#include <usb.h>
#include <optional>
#include <vector>

namespace KDM
{

/// <summary>
/// Utility class for parsing USB descriptor structures.
/// Provides static methods for extracting information from USB descriptors
/// with proper validation and bounds checking.
/// </summary>
class UsbDescriptorParser
{
public:
    // Delete constructor - this is a static utility class
    UsbDescriptorParser() = delete;

    /// <summary>
    /// Extracts the interface class from a USB configuration descriptor.
    /// Searches for the first interface descriptor and returns its bInterfaceClass.
    /// </summary>
    /// <param name="configDesc">Pointer to the USB configuration descriptor</param>
    /// <returns>The interface class if found, std::nullopt otherwise</returns>
    [[nodiscard]] static std::optional<UCHAR> ExtractInterfaceClass(
        PUSB_CONFIGURATION_DESCRIPTOR configDesc) noexcept;

    /// <summary>
    /// Validates a USB device descriptor for correctness.
    /// Checks bLength, bDescriptorType, and basic field validity.
    /// </summary>
    /// <param name="descriptor">Reference to the USB device descriptor</param>
    /// <returns>true if the descriptor is valid, false otherwise</returns>
    [[nodiscard]] static bool ValidateDeviceDescriptor(
        const USB_DEVICE_DESCRIPTOR& descriptor) noexcept;

    /// <summary>
    /// Validates a USB configuration descriptor for correctness.
    /// </summary>
    /// <param name="configDesc">Pointer to the USB configuration descriptor</param>
    /// <returns>true if the descriptor is valid, false otherwise</returns>
    [[nodiscard]] static bool ValidateConfigurationDescriptor(
        PUSB_CONFIGURATION_DESCRIPTOR configDesc) noexcept;

    /// <summary>
    /// Checks if a configuration descriptor contains any interface descriptors
    /// with string descriptors that need to be fetched.
    /// </summary>
    /// <param name="configDesc">Pointer to the USB configuration descriptor</param>
    /// <returns>true if string descriptors are present, false otherwise</returns>
    [[nodiscard]] static bool HasStringDescriptors(
        PUSB_CONFIGURATION_DESCRIPTOR configDesc) noexcept;

    /// <summary>
    /// Gets the total number of interfaces in a configuration descriptor.
    /// </summary>
    /// <param name="configDesc">Pointer to the USB configuration descriptor</param>
    /// <returns>Number of interfaces found</returns>
    [[nodiscard]] static size_t GetInterfaceCount(
        PUSB_CONFIGURATION_DESCRIPTOR configDesc) noexcept;

private:
    /// <summary>
    /// Helper to safely iterate through descriptors within a configuration.
    /// </summary>
    /// <param name="configDesc">Pointer to the USB configuration descriptor</param>
    /// <param name="commonDesc">Current descriptor position</param>
    /// <param name="descEnd">End of descriptor buffer</param>
    /// <returns>true if commonDesc is within valid bounds</returns>
    [[nodiscard]] static bool IsDescriptorInBounds(
        PUSB_COMMON_DESCRIPTOR commonDesc,
        PUCHAR descEnd) noexcept;
};

} // namespace KDM
