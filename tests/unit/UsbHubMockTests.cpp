#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <windows.h>
#include <SetupAPI.h>
#include <usb.h>
#include <usbioctl.h>
#include <usbiodef.h>
#include <wil/result.h>
#include <wil/resource.h>
#include "usbdesc.h"
#include "UsbHub.h"
#include "HubNodeInfo.h"
#include "HubConnectionInfo.h"
#include "mocks/MockDeviceCommunication.h"
#include <memory>

using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::Invoke;
using ::testing::NiceMock;

namespace KDM
{
namespace Testing
{

/// <summary>
/// Tests for UsbHub class using mock IDeviceCommunication.
/// These tests verify UsbHub behavior without requiring real USB hardware.
/// </summary>
class UsbHubMockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create NiceMock to suppress warnings for uninteresting calls
        auto mockComm = std::make_unique<NiceMock<MockDeviceCommunication>>();
        mockCommunication_ = mockComm.get();

        // Create UsbHub with injected mock
        hub_ = std::make_unique<UsbHub>(L"\\\\.\\TestHub", std::move(mockComm));
    }

    void TearDown() override
    {
        hub_.reset();
        mockCommunication_ = nullptr;
    }

    std::unique_ptr<UsbHub> hub_;
    MockDeviceCommunication* mockCommunication_ = nullptr;
};

TEST_F(UsbHubMockTest, PopulateInfo_CallsGetUsbHubNodeInformation)
{
    // Expect GetUsbHubNodeInformation to be called and set port count
    EXPECT_CALL(*mockCommunication_, GetUsbHubNodeInformation(_))
        .WillOnce(Invoke([](HubNodeInfo& nodeInfo) {
            nodeInfo.numbersOfPorts = 8;
            nodeInfo.type = L"UsbRootHub";
        }));

    // Expect EnumeratePorts to be called with port count
    EXPECT_CALL(*mockCommunication_, EnumeratePorts(testing::Eq(static_cast<ULONG>(8)), _))
        .WillOnce(Return());

    // Expect EnumeratePortsConnectionInfo to be called
    EXPECT_CALL(*mockCommunication_, EnumeratePortsConnectionInfo(testing::Eq(static_cast<ULONG>(8)), _))
        .WillOnce(Return());

    // Call PopulateInfo
    EXPECT_NO_THROW(hub_->PopulateInfo());
}

TEST_F(UsbHubMockTest, GetDeviceCommunication_ReturnsInjectedMock)
{
    // Verify that GetDeviceCommunication returns the injected mock
    IDeviceCommunication* comm = hub_->GetDeviceCommunication();
    EXPECT_EQ(comm, mockCommunication_);
}

/// <summary>
/// Tests for UsbHub class using stub IDeviceCommunication.
/// Demonstrates simpler testing without GMock.
/// </summary>
class UsbHubStubTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto stubComm = std::make_unique<StubDeviceCommunication>();
        stubCommunication_ = stubComm.get();
        stubCommunication_->SetMockPortCount(4);

        hub_ = std::make_unique<UsbHub>(L"\\\\.\\TestHub", std::move(stubComm));
    }

    void TearDown() override
    {
        hub_.reset();
        stubCommunication_ = nullptr;
    }

    std::unique_ptr<UsbHub> hub_;
    StubDeviceCommunication* stubCommunication_ = nullptr;
};

TEST_F(UsbHubStubTest, PopulateInfo_WorksWithStub)
{
    // PopulateInfo should work with stub implementation
    EXPECT_NO_THROW(hub_->PopulateInfo());
}

TEST_F(UsbHubStubTest, GetDeviceCommunication_ReturnsInjectedStub)
{
    IDeviceCommunication* comm = hub_->GetDeviceCommunication();
    EXPECT_EQ(comm, stubCommunication_);
}

TEST_F(UsbHubStubTest, StubReturnsConfiguredPortCount)
{
    // Configure stub with specific port count
    stubCommunication_->SetMockPortCount(12);

    HubNodeInfo nodeInfo;
    stubCommunication_->GetUsbHubNodeInformation(nodeInfo);

    EXPECT_EQ(nodeInfo.numbersOfPorts, 12);
}

TEST_F(UsbHubStubTest, StubReturnsConfiguredHubType)
{
    stubCommunication_->SetMockHubType(L"CustomHubType");

    HubNodeInfo nodeInfo;
    stubCommunication_->GetUsbHubNodeInformation(nodeInfo);

    EXPECT_EQ(nodeInfo.type, L"CustomHubType");
}

TEST_F(UsbHubStubTest, StubEnumeratePortsCreatesEntries)
{
    std::map<size_t, HubPortInfo> portProps;
    stubCommunication_->EnumeratePorts(5, portProps);

    EXPECT_EQ(portProps.size(), 5);

    // Verify all ports are present (1-indexed)
    for (size_t i = 1; i <= 5; ++i)
    {
        EXPECT_TRUE(portProps.count(i) > 0) << "Port " << i << " should exist";
    }
}

TEST_F(UsbHubStubTest, StubEnumeratePortsConnectionInfoCreatesEntries)
{
    std::map<size_t, HubConnectionInfo> connInfo;
    stubCommunication_->EnumeratePortsConnectionInfo(3, connInfo);

    EXPECT_EQ(connInfo.size(), 3);

    for (const auto& [index, info] : connInfo)
    {
        EXPECT_EQ(info._connectionIndex, index);
        EXPECT_EQ(info._connectionStatus, NoDeviceConnected);
    }
}

/// <summary>
/// Integration test demonstrating mock usage for simulating connected devices.
/// </summary>
TEST(UsbHubDependencyInjection, CanInjectMockToSimulateConnectedDevice)
{
    auto mockComm = std::make_unique<NiceMock<MockDeviceCommunication>>();
    NiceMock<MockDeviceCommunication>* mockPtr = mockComm.get();

    // Set up expectations for a hub with one connected device
    EXPECT_CALL(*mockPtr, GetUsbHubNodeInformation(_))
        .WillOnce(Invoke([](HubNodeInfo& nodeInfo) {
            nodeInfo.numbersOfPorts = 2;
            nodeInfo.type = L"UsbRootHub30";
        }));

    EXPECT_CALL(*mockPtr, EnumeratePorts(testing::Eq(static_cast<ULONG>(2)), _))
        .WillOnce(Return());

    EXPECT_CALL(*mockPtr, EnumeratePortsConnectionInfo(testing::Eq(static_cast<ULONG>(2)), _))
        .WillOnce(Invoke([](ULONG /*numberOfPorts*/, std::map<size_t, HubConnectionInfo>& connInfo) {
            connInfo.clear();

            // Port 1: connected USB flash drive
            HubConnectionInfo port1;
            port1._connectionIndex = 1;
            port1._connectionStatus = DeviceConnected;
            port1._deviceDescriptor.idVendor = 0x0951;  // Kingston
            port1._deviceDescriptor.idProduct = 0x172B; // DataTraveler
            port1._deviceDescriptor.bDeviceClass = 0x00;
            port1._speed = 3; // SuperSpeed
            port1._deviceIsHub = FALSE;
            connInfo[1] = port1;

            // Port 2: empty
            HubConnectionInfo port2;
            port2._connectionIndex = 2;
            port2._connectionStatus = NoDeviceConnected;
            connInfo[2] = port2;
        }));

    // Create UsbHub with mock
    UsbHub hub(L"\\\\.\\MockedHub", std::move(mockComm));

    // PopulateInfo should succeed with mocked data
    EXPECT_NO_THROW(hub.PopulateInfo());
}

/// <summary>
/// Test demonstrating that UsbHub can be constructed with default behavior.
/// Note: This test requires real USB hardware and will be skipped if no hub is found.
/// </summary>
TEST(UsbHubDependencyInjection, DefaultConstructorUsesRealCommunication)
{
    // This test verifies the default constructor path still works
    // It will fail to open an invalid path, demonstrating the real implementation is used
    EXPECT_THROW(
        {
            UsbHub hub(L"\\\\.\\InvalidTestPath");
        },
        std::exception
    );
}

} // namespace Testing
} // namespace KDM
