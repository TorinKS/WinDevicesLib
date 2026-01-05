#pragma once

#include <Windows.h>
#include <usb.h>
#include <map>
#include <string>
#include "usbdesc.h"

namespace KDM
{
    // Forward declarations
    struct HubNodeInfo;
    struct HubNodeInfoEx;
    struct HubNodeCapabilitiesEx;
    struct HubPortInfo;
    struct HubConnectionInfo;

    /// <summary>
    /// Interface for device communication operations.
    /// Enables dependency injection and mock implementations for unit testing.
    /// </summary>
    class IDeviceCommunication
    {
    public:
        virtual ~IDeviceCommunication() = default;

        // Prevent copying (interface should be used via pointer/reference)
        IDeviceCommunication(const IDeviceCommunication&) = delete;
        IDeviceCommunication& operator=(const IDeviceCommunication&) = delete;

        // Allow moving for derived classes
        IDeviceCommunication(IDeviceCommunication&&) noexcept = default;
        IDeviceCommunication& operator=(IDeviceCommunication&&) noexcept = default;

        // IOCTL_USB_GET_NODE_INFORMATION
        virtual void GetUsbHubNodeInformation(HubNodeInfo& nodeInfo) = 0;
        // USB_HUB_INFORMATION_EX
        virtual void GetUsbHubNodeInformationEx(HubNodeInfoEx& nodeInfo) = 0;
        // USB_HUB_CAPABILITIES_EX
        virtual void GetUsbHubNodeCapabilitiesEx(HubNodeCapabilitiesEx& nodeInfo) = 0;
        // IOCTL_USB_GET_NODE_CONNECTION_NAME - get info about an external hub name
        virtual void GetUsbExternalHubName(DWORD index, std::wstring& hubName) = 0;

        virtual void EnumeratePorts(ULONG numberOfPorts, std::map<size_t, HubPortInfo>& portConnectorPropsList) = 0;
        virtual void EnumeratePortsConnectionInfo(ULONG numberOfPorts,
            std::map<size_t, HubConnectionInfo>& hubConnectionInfoList) = 0;

        [[nodiscard]] virtual std::wstring GetDriverKeyName(ULONG connectionIndex) = 0;
        [[nodiscard]] virtual PUSB_DESCRIPTOR_REQUEST GetConfigDescriptor(ULONG connectionIndex, UCHAR descriptorIndex) = 0;
        [[nodiscard]] virtual PSTRING_DESCRIPTOR_NODE GetStringDescriptor(ULONG connectionIndex, UCHAR descriptorIndex, USHORT languageId) = 0;

        [[nodiscard]] virtual HANDLE GetFileHandle() = 0;

    protected:
        // Protected default constructor - only derived classes can instantiate
        IDeviceCommunication() = default;
    };

}
