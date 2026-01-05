#pragma once

#include "IDeviceCommunication.h"

namespace KDM
{
	struct HubNodeInfo;
	struct HubNodeInfoEx;
	struct HubNodeCapabilitiesEx;
	struct HubPortInfo;
	struct HubConnectionInfo;

	/// @brief Handles low-level USB hub communication via Windows IOCTL calls.
	///
	/// This class provides an interface for querying USB hub information and
	/// enumerating connected devices through DeviceIoControl operations.
	/// It uses WIL's unique_hfile for RAII-based file handle management.
	///
	/// Supported IOCTLs:
	/// - IOCTL_USB_GET_NODE_INFORMATION: Hub node info (port count, type)
	/// - IOCTL_USB_GET_HUB_INFORMATION_EX: Extended hub info (Windows 8+)
	/// - IOCTL_USB_GET_HUB_CAPABILITIES_EX: Hub capabilities (root hub detection)
	/// - IOCTL_USB_GET_NODE_CONNECTION_INFORMATION[_EX][_V2]: Port connection info
	/// - IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME: Driver key for connected device
	/// - IOCTL_USB_GET_NODE_CONNECTION_NAME: External hub name
	/// - IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION: USB descriptors
	/// - IOCTL_USB_GET_PORT_CONNECTOR_PROPERTIES: USB 3.0 port properties
	///
	/// @note This class is move-only due to managing a unique file handle.
	/// Implements IDeviceCommunication for dependency injection and testing.
	///
	/// @example
	/// @code
	/// DeviceCommunication comm(L"\\\\.\\USB#ROOT_HUB30#...");
	/// HubNodeInfo hubInfo;
	/// comm.GetUsbHubNodeInformation(hubInfo);
	/// std::map<size_t, HubConnectionInfo> connections;
	/// comm.EnumeratePortsConnectionInfo(hubInfo.numbersOfPorts, connections);
	/// @endcode
	class DeviceCommunication : public IDeviceCommunication
	{
	public:
		/// @brief Opens a handle to the specified USB hub device.
		/// @param devicePath Windows device path (e.g., "\\\\.\\USB#ROOT_HUB30#...").
		/// @throws wil::ResultException if CreateFileW fails.
		explicit DeviceCommunication(std::wstring devicePath);

		~DeviceCommunication() override = default;

		// Move-only semantics (unique_hfile cannot be copied)
		DeviceCommunication(const DeviceCommunication&) = delete;
		DeviceCommunication& operator=(const DeviceCommunication&) = delete;
		DeviceCommunication(DeviceCommunication&&) noexcept = default;
		DeviceCommunication& operator=(DeviceCommunication&&) noexcept = default;

		/// @brief Retrieves basic hub node information (port count, hub type).
		/// @param[out] nodeInfo Receives hub information.
		/// @throws wil::ResultException if IOCTL_USB_GET_NODE_INFORMATION fails.
		void GetUsbHubNodeInformation(HubNodeInfo& nodeInfo) override;

		/// @brief Retrieves extended hub information (Windows 8+).
		/// @param[out] nodeInfo Receives extended hub info. Check _isHubInfoExSupport
		///             to determine if the data is valid (not supported on older Windows).
		void GetUsbHubNodeInformationEx(HubNodeInfoEx& nodeInfo) override;

		/// @brief Retrieves hub capability flags (e.g., root hub detection).
		/// @param[out] nodeInfo Receives capability information.
		/// @throws wil::ResultException if IOCTL not supported on this OS version.
		void GetUsbHubNodeCapabilitiesEx(HubNodeCapabilitiesEx& nodeInfo) override;

		/// @brief Retrieves the symbolic link name of an external hub.
		/// @param index Connection index (port number) where the hub is connected.
		/// @param[out] hubName Receives the hub's symbolic link name.
		/// @throws wil::ResultException on IOCTL failure.
		void GetUsbExternalHubName(DWORD index, std::wstring& hubName) override;

		/// @brief Enumerates USB 3.0+ port connector properties.
		/// @param numberOfPorts Number of ports to enumerate (1 to numberOfPorts).
		/// @param[out] portConnectorPropsList Map from port number to port properties.
		/// @throws InvalidDeviceArgumentException if numberOfPorts is 0 or exceeds limit.
		/// @throws InvalidDeviceHandleException if device handle is invalid.
		void EnumeratePorts(ULONG numberOfPorts,
			std::map<size_t, HubPortInfo>& portConnectorPropsList) override;

		/// @brief Enumerates connection information for all hub ports.
		///
		/// For each port, queries:
		/// - Connection status (device connected, not connected, etc.)
		/// - Device descriptor (VID, PID, device class, etc.)
		/// - USB speed (with USB 3.0+ SuperSpeed detection via V2 IOCTL)
		/// - Driver key name for connected devices
		///
		/// @param numberOfPorts Number of ports to enumerate.
		/// @param[out] hubConnectionInfoList Map from port number to connection info.
		/// @throws InvalidDeviceArgumentException if numberOfPorts is 0 or exceeds limit.
		/// @throws InvalidDeviceHandleException if device handle is invalid.
		void EnumeratePortsConnectionInfo(ULONG numberOfPorts,
			std::map<size_t, HubConnectionInfo>& hubConnectionInfoList) override;

		/// @brief Retrieves the driver key name for a device connected at the specified port.
		/// @param connectionIndex Port number (1-based index).
		/// @return Driver key name string.
		/// @throws InvalidDeviceArgumentException if connectionIndex is 0.
		/// @throws InvalidDeviceHandleException if device handle is invalid.
		/// @throws wil::ResultException on IOCTL failure.
		[[nodiscard]] std::wstring GetDriverKeyName(ULONG connectionIndex) override;

		/// @brief Retrieves the configuration descriptor for a connected device.
		///
		/// Uses a two-phase query: first gets descriptor header to determine size,
		/// then allocates and retrieves the full descriptor.
		///
		/// @param connectionIndex Port number where device is connected.
		/// @param descriptorIndex Configuration descriptor index (usually 0).
		/// @return Pointer to USB_DESCRIPTOR_REQUEST containing the descriptor.
		///         Caller is responsible for freeing the memory.
		/// @throws InvalidDeviceArgumentException if connectionIndex is 0.
		[[nodiscard]] PUSB_DESCRIPTOR_REQUEST GetConfigDescriptor(
			ULONG connectionIndex, UCHAR descriptorIndex) override;

		/// @brief Retrieves a string descriptor from a connected device.
		/// @param connectionIndex Port number where device is connected.
		/// @param descriptorIndex String descriptor index.
		/// @param languageId Language ID for the string (e.g., 0x0409 for English).
		/// @return Pointer to STRING_DESCRIPTOR_NODE containing the string.
		///         Caller is responsible for freeing the memory.
		[[nodiscard]] PSTRING_DESCRIPTOR_NODE GetStringDescriptor(
			ULONG connectionIndex, UCHAR descriptorIndex, USHORT languageId) override;

		/// @brief Returns the underlying file handle.
		/// @return The device file handle.
		/// @note The handle is owned by this object; do not close it.
		[[nodiscard]] HANDLE GetFileHandle() override;

	private:
		wil::unique_hfile _hFile;
	};
}
