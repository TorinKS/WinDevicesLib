#include <gtest/gtest.h>
#include <windows.h>
#include <SetupAPI.h>
#include <usbioctl.h>
#include <usb.h>
#include <usbiodef.h>
#include <wil/result.h>
#include <wil/resource.h>
#include "usbdesc.h"
#include "DeviceCommunication.h"
#include "DevicesManager.h"
#include "HubConnectionInfo.h"
#include "HubNodeInfo.h"
#include "Exceptions.h"
#include "UtilConvert.h"
#include <memory>
#include <map>
#include <set>
#include <iostream>
#include <vector>

/*
 * Tests for DeviceCommunication class
 *
 * Note: These tests interact with real USB hardware on the system.
 * They are designed to work on any Windows system with USB devices.
 */

class DeviceCommunicationTest : public ::testing::Test {
protected:
    std::unique_ptr<KDM::DevicesManager> manager;

    void SetUp() override {
        manager = std::make_unique<KDM::DevicesManager>();
    }

    void TearDown() override {
        manager.reset();
    }

    // Helper to find a USB hub device path using SetupAPI
    std::wstring FindUsbHubDevicePath() {
        // Use SetupAPI to enumerate USB hubs (including root hubs)
        // GUID_DEVINTERFACE_USB_HUB is defined in usbiodef.h
        HDEVINFO deviceInfo = SetupDiGetClassDevs(
            &GUID_DEVINTERFACE_USB_HUB,
            NULL,
            NULL,
            DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
        );

        if (deviceInfo == INVALID_HANDLE_VALUE) {
            return L"";
        }

        // Ensure cleanup of device info handle
        auto cleanup = wil::scope_exit([&] {
            SetupDiDestroyDeviceInfoList(deviceInfo);
        });

        // Enumerate all hub devices
        SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &GUID_DEVINTERFACE_USB_HUB, i, &deviceInterfaceData); i++) {
            // Get the required buffer size
            DWORD requiredSize = 0;
            SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, NULL, 0, &requiredSize, NULL);

            if (requiredSize == 0) {
                continue;
            }

            // Allocate buffer for device interface detail
            std::vector<BYTE> buffer(requiredSize);
            PSP_DEVICE_INTERFACE_DETAIL_DATA detailData = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(buffer.data());
            detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

            // Get the device interface detail
            if (!SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, detailData, requiredSize, NULL, NULL)) {
                continue;
            }

            std::wstring devicePath = detailData->DevicePath;

            // Try to open and verify it's a valid hub with ports
            try {
                KDM::DeviceCommunication comm(devicePath);
                KDM::HubNodeInfo nodeInfo;
                comm.GetUsbHubNodeInformation(nodeInfo);

                if (nodeInfo.numbersOfPorts > 0) {
                    return devicePath;
                }
            } catch (...) {
                // Not a valid hub or can't open, continue to next
                continue;
            }
        }

        return L"";
    }
};

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_NoThrow) {
    // Find a USB hub on the system
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system - test requires USB hub";
        return;
    }

    // std::cout << "Testing with hub: " << KDM::UtilConvert::WStringToUTF8(hubPath) << std::endl;

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;

    // Should not throw when enumerating ports
    EXPECT_NO_THROW(comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo));
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_PopulatesMap) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    ASSERT_GT(nodeInfo.numbersOfPorts, 0) << "Hub should have at least one port";

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    // Map should have entries for each port
    EXPECT_EQ(connectionInfo.size(), nodeInfo.numbersOfPorts)
        << "Should have connection info for each port";
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_ConnectionIndexSet) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    // Verify connection indices are set correctly
    for (const auto& [index, info] : connectionInfo) {
        EXPECT_EQ(info._connectionIndex, index)
            << "Connection index should match map key";
        EXPECT_GE(info._connectionIndex, 1)
            << "Connection index should be 1-based";
        EXPECT_LE(info._connectionIndex, nodeInfo.numbersOfPorts)
            << "Connection index should not exceed port count";
    }
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_ConnectionStatus) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    // Check connection status for all ports
    int connectedCount = 0;
    int noDeviceCount = 0;

    for (const auto& [index, info] : connectionInfo) {
        // Connection status should be one of the valid USB_CONNECTION_STATUS values
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/usbioctl/ne-usbioctl-_usb_connection_status
        EXPECT_GE(info._connectionStatus, 0) << "Connection status should be valid enum value";
        EXPECT_LE(info._connectionStatus, 6) << "Connection status should be valid enum value";

        if (info._connectionStatus == DeviceConnected) {
            connectedCount++;
        } else if (info._connectionStatus == NoDeviceConnected) {
            noDeviceCount++;
        }
    }

    // std::cout << "Ports: " << nodeInfo.numbersOfPorts
    //           << " | Connected: " << connectedCount
    //           << " | Empty: " << noDeviceCount << std::endl;

    // Total ports should equal connected + no device + other statuses
    EXPECT_EQ(connectedCount + noDeviceCount, connectionInfo.size());
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_ConnectedDeviceHasDescriptor) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    bool foundConnectedDevice = false;

    for (const auto& [index, info] : connectionInfo) {
        if (info._connectionStatus == DeviceConnected) {
            foundConnectedDevice = true;

            // Connected device should have valid descriptor
            EXPECT_GT(info._deviceDescriptor.bLength, 0)
                << "Device descriptor length should be > 0 for port " << index;
            EXPECT_EQ(info._deviceDescriptor.bDescriptorType, USB_DEVICE_DESCRIPTOR_TYPE)
                << "Descriptor type should be USB_DEVICE_DESCRIPTOR_TYPE (0x01)";
            EXPECT_GT(info._deviceDescriptor.bcdUSB, 0)
                << "USB version should be > 0 for port " << index;

            // Should have vendor/product ID
            // Note: Some devices might have VID/PID of 0, but that's rare
            // std::cout << "Port " << index << ": VID="
            //           << std::hex << info._deviceDescriptor.idVendor
            //           << " PID=" << info._deviceDescriptor.idProduct
            //           << std::dec << std::endl;
        }
    }

    if (!foundConnectedDevice) {
        GTEST_SKIP() << "No connected devices found on hub - test needs connected USB device";
    }
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_SpeedDetection) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    for (const auto& [index, info] : connectionInfo) {
        if (info._connectionStatus == DeviceConnected) {
            // Speed should be one of the USB speed values
            // 0 = Low Speed, 1 = Full Speed, 2 = High Speed, 3 = Super Speed, etc.
            EXPECT_GE(info._speed, 0) << "Speed should be >= 0 for port " << index;
            EXPECT_LE(info._speed, 4) << "Speed should be <= 4 (max known speed) for port " << index;

            // const char* speedName[] = {"Low", "Full", "High", "Super", "SuperPlus"};
            // if (info._speed <= 4) {
            //     std::cout << "Port " << index << " speed: " << speedName[info._speed] << std::endl;
            // }
        }
    }
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_DeviceIsHubFlag) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    for (const auto& [index, info] : connectionInfo) {
        if (info._connectionStatus == DeviceConnected) {
            // DeviceIsHub is a BOOLEAN (0 or 1)
            EXPECT_GE(info._deviceIsHub, 0) << "DeviceIsHub should be boolean for port " << index;
            EXPECT_LE(info._deviceIsHub, 1) << "DeviceIsHub should be boolean for port " << index;

            // if (info._deviceIsHub) {
            //     std::cout << "Port " << index << " has an external hub connected" << std::endl;
            // }
        }
    }
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_DeviceAddress) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    std::set<USHORT> usedAddresses;

    for (const auto& [index, info] : connectionInfo) {
        if (info._connectionStatus == DeviceConnected) {
            // Device address should be 1-127 (USB spec)
            EXPECT_GE(info._deviceAddress, 1)
                << "Device address should be >= 1 for port " << index;
            EXPECT_LE(info._deviceAddress, 127)
                << "Device address should be <= 127 (USB spec) for port " << index;

            // Each device should have unique address
            EXPECT_EQ(usedAddresses.count(info._deviceAddress), 0)
                << "Device address " << info._deviceAddress << " already in use";
            usedAddresses.insert(info._deviceAddress);

            // std::cout << "Port " << index << " device address: " << info._deviceAddress << std::endl;
        }
    }
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_PipeInformation) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    for (const auto& [index, info] : connectionInfo) {
        if (info._connectionStatus == DeviceConnected) {
            // Number of open pipes should match pipe list size
            EXPECT_EQ(info._numberOfOpenPipes, info._pipeList.size())
                << "Pipe count mismatch for port " << index;

            if (info._numberOfOpenPipes > 0) {
                // std::cout << "Port " << index << " has " << info._numberOfOpenPipes
                //           << " open pipes" << std::endl;

                // Verify pipe information is valid
                for (size_t i = 0; i < info._pipeList.size(); i++) {
                    const auto& pipe = info._pipeList[i];

                    // Endpoint address should have valid direction bit
                    EXPECT_GE(pipe.EndpointDescriptor.bEndpointAddress, 0);
                    EXPECT_LE(pipe.EndpointDescriptor.bEndpointAddress, 0xFF);

                    // Transfer type should be valid (Control, Iso, Bulk, Interrupt)
                    UCHAR transferType = pipe.EndpointDescriptor.bmAttributes & 0x03;
                    EXPECT_LE(transferType, 3) << "Invalid transfer type";
                }
            }
        }
    }
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_CurrentConfiguration) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    for (const auto& [index, info] : connectionInfo) {
        if (info._connectionStatus == DeviceConnected) {
            // Current configuration value (0 = unconfigured, 1+ = configured)
            EXPECT_GE(info._currentConfigurationValue, 0)
                << "Configuration value should be >= 0 for port " << index;

            // Most devices will be configured (value >= 1)
            // if (info._currentConfigurationValue > 0) {
            //     std::cout << "Port " << index << " using configuration "
            //               << static_cast<int>(info._currentConfigurationValue) << std::endl;
            // }
        }
    }
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_DriverKeyName) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    int devicesWithDrivers = 0;

    for (const auto& [index, info] : connectionInfo) {
        if (info._connectionStatus == DeviceConnected) {
            // Connected devices should have driver key names
            // (unless the driver failed to load)
            if (!info._driverKeyName.empty()) {
                devicesWithDrivers++;

                // Driver key name should contain typical registry key pattern
                EXPECT_TRUE(info._driverKeyName.find(L"{") != std::wstring::npos ||
                           info._driverKeyName.find(L"\\") != std::wstring::npos)
                    << "Driver key name should look like registry path";

                // std::wcout << L"Port " << index << L" driver: "
                //           << info._driverKeyName << std::endl;
            }
        }
    }

    // std::cout << "Found " << devicesWithDrivers << " devices with loaded drivers" << std::endl;
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_ZeroPorts) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;

    // Edge case: enumerate with 0 ports should throw InvalidDeviceArgumentException
    EXPECT_THROW(comm.EnumeratePortsConnectionInfo(0, connectionInfo), KDM::InvalidDeviceArgumentException);
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_MultipleCalls) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo1;
    std::map<size_t, KDM::HubConnectionInfo> connectionInfo2;

    // Call twice
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo1);
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo2);

    // Both calls should produce same results
    EXPECT_EQ(connectionInfo1.size(), connectionInfo2.size())
        << "Multiple calls should produce consistent results";

    for (const auto& [index, info1] : connectionInfo1) {
        ASSERT_TRUE(connectionInfo2.count(index) > 0) << "Port " << index << " missing in second call";

        const auto& info2 = connectionInfo2.at(index);

        // Connection status should match
        EXPECT_EQ(info1._connectionStatus, info2._connectionStatus)
            << "Connection status mismatch for port " << index;

        if (info1._connectionStatus == DeviceConnected) {
            // Device descriptor should match
            EXPECT_EQ(info1._deviceDescriptor.idVendor, info2._deviceDescriptor.idVendor);
            EXPECT_EQ(info1._deviceDescriptor.idProduct, info2._deviceDescriptor.idProduct);
            EXPECT_EQ(info1._speed, info2._speed);
            EXPECT_EQ(info1._deviceAddress, info2._deviceAddress);
        }
    }
}

TEST_F(DeviceCommunicationTest, EnumeratePortsConnectionInfo_ClearsExistingMap) {
    std::wstring hubPath = FindUsbHubDevicePath();

    if (hubPath.empty()) {
        GTEST_SKIP() << "No USB hub found on system";
        return;
    }

    KDM::DeviceCommunication comm(hubPath);
    KDM::HubNodeInfo nodeInfo;
    comm.GetUsbHubNodeInformation(nodeInfo);

    std::map<size_t, KDM::HubConnectionInfo> connectionInfo;

    // Add some dummy data
    KDM::HubConnectionInfo dummy;
    dummy._connectionIndex = 9999;
    connectionInfo[9999] = dummy;

    ASSERT_EQ(connectionInfo.size(), 1) << "Should have dummy entry";

    // Enumerate - should clear the map first
    comm.EnumeratePortsConnectionInfo(nodeInfo.numbersOfPorts, connectionInfo);

    // Dummy entry should be gone
    EXPECT_EQ(connectionInfo.count(9999), 0) << "Map should be cleared before enumeration";
    EXPECT_EQ(connectionInfo.size(), nodeInfo.numbersOfPorts)
        << "Should only have real port entries";
}
