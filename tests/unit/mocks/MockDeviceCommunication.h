#pragma once

#include "IDeviceCommunication.h"
#include "HubNodeInfo.h"
#include "HubNodeInfoEx.h"
#include "HubNodeCapabilitiesEx.h"
#include "HubPortInfo.h"
#include "HubConnectionInfo.h"
#include <gmock/gmock.h>

namespace KDM
{
namespace Testing
{

// Type aliases to avoid comma issues in MOCK_METHOD macros
using HubPortInfoMap = std::map<size_t, HubPortInfo>;
using HubConnectionInfoMap = std::map<size_t, HubConnectionInfo>;

/// <summary>
/// Mock implementation of IDeviceCommunication for unit testing.
/// Allows testing of UsbHub and other classes without requiring real USB hardware.
/// </summary>
class MockDeviceCommunication : public IDeviceCommunication
{
public:
    MockDeviceCommunication() = default;
    ~MockDeviceCommunication() override = default;

    // GMock method declarations
    MOCK_METHOD(void, GetUsbHubNodeInformation, (HubNodeInfo& nodeInfo), (override));
    MOCK_METHOD(void, GetUsbHubNodeInformationEx, (HubNodeInfoEx& nodeInfo), (override));
    MOCK_METHOD(void, GetUsbHubNodeCapabilitiesEx, (HubNodeCapabilitiesEx& nodeInfo), (override));
    MOCK_METHOD(void, GetUsbExternalHubName, (DWORD index, std::wstring& hubName), (override));
    MOCK_METHOD(void, EnumeratePorts, (ULONG numberOfPorts, HubPortInfoMap& portConnectorPropsList), (override));
    MOCK_METHOD(void, EnumeratePortsConnectionInfo, (ULONG numberOfPorts, HubConnectionInfoMap& hubConnectionInfoList), (override));
    MOCK_METHOD(std::wstring, GetDriverKeyName, (ULONG connectionIndex), (override));
    MOCK_METHOD(PUSB_DESCRIPTOR_REQUEST, GetConfigDescriptor, (ULONG connectionIndex, UCHAR descriptorIndex), (override));
    MOCK_METHOD(PSTRING_DESCRIPTOR_NODE, GetStringDescriptor, (ULONG connectionIndex, UCHAR descriptorIndex, USHORT languageId), (override));
    MOCK_METHOD(HANDLE, GetFileHandle, (), (override));
};

/// <summary>
/// Simple stub implementation for basic testing without GMock dependency.
/// Provides configurable return values for common scenarios.
/// </summary>
class StubDeviceCommunication : public IDeviceCommunication
{
public:
    StubDeviceCommunication() = default;
    ~StubDeviceCommunication() override = default;

    // Configuration methods
    void SetMockPortCount(ULONG count) { mockPortCount_ = count; }
    void SetMockHubType(const std::wstring& type) { mockHubType_ = type; }
    void SetMockFileHandle(HANDLE handle) { mockFileHandle_ = handle; }

    // IDeviceCommunication implementation
    void GetUsbHubNodeInformation(HubNodeInfo& nodeInfo) override
    {
        nodeInfo.numbersOfPorts = mockPortCount_;
        nodeInfo.type = mockHubType_;
    }

    void GetUsbHubNodeInformationEx(HubNodeInfoEx& nodeInfo) override
    {
        nodeInfo._highestPortNumber = static_cast<USHORT>(mockPortCount_);
        nodeInfo._isHubInfoExSupport = true;
    }

    void GetUsbHubNodeCapabilitiesEx(HubNodeCapabilitiesEx& /*nodeInfo*/) override
    {
        // Default stub - does nothing
    }

    void GetUsbExternalHubName(DWORD /*index*/, std::wstring& hubName) override
    {
        hubName = L"";
    }

    void EnumeratePorts(ULONG numberOfPorts, std::map<size_t, HubPortInfo>& portConnectorPropsList) override
    {
        portConnectorPropsList.clear();
        for (ULONG i = 1; i <= numberOfPorts; ++i)
        {
            HubPortInfo portInfo;
            portConnectorPropsList.try_emplace(i, std::move(portInfo));
        }
    }

    void EnumeratePortsConnectionInfo(ULONG numberOfPorts, std::map<size_t, HubConnectionInfo>& hubConnectionInfoList) override
    {
        hubConnectionInfoList.clear();
        for (ULONG i = 1; i <= numberOfPorts; ++i)
        {
            HubConnectionInfo connInfo;
            connInfo._connectionIndex = i;
            connInfo._connectionStatus = NoDeviceConnected;
            hubConnectionInfoList.try_emplace(i, std::move(connInfo));
        }
    }

    [[nodiscard]] std::wstring GetDriverKeyName(ULONG /*connectionIndex*/) override
    {
        return L"";
    }

    [[nodiscard]] PUSB_DESCRIPTOR_REQUEST GetConfigDescriptor(ULONG /*connectionIndex*/, UCHAR /*descriptorIndex*/) override
    {
        return nullptr;
    }

    [[nodiscard]] PSTRING_DESCRIPTOR_NODE GetStringDescriptor(ULONG /*connectionIndex*/, UCHAR /*descriptorIndex*/, USHORT /*languageId*/) override
    {
        return nullptr;
    }

    [[nodiscard]] HANDLE GetFileHandle() override
    {
        return mockFileHandle_;
    }

private:
    ULONG mockPortCount_ = 4;
    std::wstring mockHubType_ = L"UsbHub";
    HANDLE mockFileHandle_ = INVALID_HANDLE_VALUE;
};

} // namespace Testing
} // namespace KDM
