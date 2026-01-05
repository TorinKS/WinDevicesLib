#include "pch.h"
#include "UsbDescriptorParser.h"

namespace KDM
{

std::optional<UCHAR> UsbDescriptorParser::ExtractInterfaceClass(
    PUSB_CONFIGURATION_DESCRIPTOR configDesc) noexcept
{
    if (!ValidateConfigurationDescriptor(configDesc))
    {
        return std::nullopt;
    }

    PUCHAR descEnd = reinterpret_cast<PUCHAR>(configDesc) + configDesc->wTotalLength;
    PUSB_COMMON_DESCRIPTOR commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(configDesc);

    // Move past the configuration descriptor
    commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(
        reinterpret_cast<PUCHAR>(commonDesc) + commonDesc->bLength);

    while (IsDescriptorInBounds(commonDesc, descEnd))
    {
        if (commonDesc->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE)
        {
            PUSB_INTERFACE_DESCRIPTOR interfaceDesc =
                reinterpret_cast<PUSB_INTERFACE_DESCRIPTOR>(commonDesc);
            return interfaceDesc->bInterfaceClass;
        }
        commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(
            reinterpret_cast<PUCHAR>(commonDesc) + commonDesc->bLength);
    }

    return std::nullopt;
}

bool UsbDescriptorParser::ValidateDeviceDescriptor(
    const USB_DEVICE_DESCRIPTOR& descriptor) noexcept
{
    // Check descriptor length
    if (descriptor.bLength != sizeof(USB_DEVICE_DESCRIPTOR))
    {
        return false;
    }

    // Check descriptor type
    if (descriptor.bDescriptorType != USB_DEVICE_DESCRIPTOR_TYPE)
    {
        return false;
    }

    // USB version should be at least 1.0 (0x0100)
    if (descriptor.bcdUSB < 0x0100)
    {
        return false;
    }

    return true;
}

bool UsbDescriptorParser::ValidateConfigurationDescriptor(
    PUSB_CONFIGURATION_DESCRIPTOR configDesc) noexcept
{
    if (configDesc == nullptr)
    {
        return false;
    }

    // Check minimum length
    if (configDesc->bLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        return false;
    }

    // Check descriptor type
    if (configDesc->bDescriptorType != USB_CONFIGURATION_DESCRIPTOR_TYPE)
    {
        return false;
    }

    // Total length should be at least the configuration descriptor size
    if (configDesc->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        return false;
    }

    return true;
}

bool UsbDescriptorParser::HasStringDescriptors(
    PUSB_CONFIGURATION_DESCRIPTOR configDesc) noexcept
{
    if (!ValidateConfigurationDescriptor(configDesc))
    {
        return false;
    }

    PUCHAR descEnd = reinterpret_cast<PUCHAR>(configDesc) + configDesc->wTotalLength;
    PUSB_COMMON_DESCRIPTOR commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(configDesc);

    // Move past the configuration descriptor
    commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(
        reinterpret_cast<PUCHAR>(commonDesc) + commonDesc->bLength);

    while (IsDescriptorInBounds(commonDesc, descEnd))
    {
        switch (commonDesc->bDescriptorType)
        {
        case USB_CONFIGURATION_DESCRIPTOR_TYPE:
            if (reinterpret_cast<PUSB_CONFIGURATION_DESCRIPTOR>(commonDesc)->iConfiguration)
            {
                return true;
            }
            break;

        case USB_INTERFACE_DESCRIPTOR_TYPE:
            // Validate interface descriptor length
            if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
            {
                return false;
            }
            if (reinterpret_cast<PUSB_INTERFACE_DESCRIPTOR>(commonDesc)->iInterface)
            {
                return true;
            }
            break;

        default:
            break;
        }

        commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(
            reinterpret_cast<PUCHAR>(commonDesc) + commonDesc->bLength);
    }

    return false;
}

size_t UsbDescriptorParser::GetInterfaceCount(
    PUSB_CONFIGURATION_DESCRIPTOR configDesc) noexcept
{
    if (!ValidateConfigurationDescriptor(configDesc))
    {
        return 0;
    }

    size_t count = 0;
    PUCHAR descEnd = reinterpret_cast<PUCHAR>(configDesc) + configDesc->wTotalLength;
    PUSB_COMMON_DESCRIPTOR commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(configDesc);

    // Move past the configuration descriptor
    commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(
        reinterpret_cast<PUCHAR>(commonDesc) + commonDesc->bLength);

    while (IsDescriptorInBounds(commonDesc, descEnd))
    {
        if (commonDesc->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE)
        {
            ++count;
        }
        commonDesc = reinterpret_cast<PUSB_COMMON_DESCRIPTOR>(
            reinterpret_cast<PUCHAR>(commonDesc) + commonDesc->bLength);
    }

    return count;
}

bool UsbDescriptorParser::IsDescriptorInBounds(
    PUSB_COMMON_DESCRIPTOR commonDesc,
    PUCHAR descEnd) noexcept
{
    if (commonDesc == nullptr || descEnd == nullptr)
    {
        return false;
    }

    PUCHAR currentPos = reinterpret_cast<PUCHAR>(commonDesc);

    // Check if we have room for the common descriptor header
    if (currentPos + sizeof(USB_COMMON_DESCRIPTOR) > descEnd)
    {
        return false;
    }

    // Check if the full descriptor fits
    if (currentPos + commonDesc->bLength > descEnd)
    {
        return false;
    }

    // Zero-length descriptors are invalid and would cause infinite loops
    if (commonDesc->bLength == 0)
    {
        return false;
    }

    return true;
}

} // namespace KDM
