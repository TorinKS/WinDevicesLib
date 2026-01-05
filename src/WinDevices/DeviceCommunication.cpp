#include "pch.h"
#include "HubNodeInfo.h"
#include "HubNodeInfoEx.h"
#include "HubNodeCapabilitiesEx.h"
#include "HubPortInfo.h"
#include "HubConnectionInfo.h"
#include "DeviceCommunication.h"
#include "UsbClassCodes.h"
#include <spdlog/spdlog.h>
#include <utility>
#include <cstring>

namespace KDM
{
	namespace
	{
		// Helper to map USB hub node type to string representation
		constexpr std::wstring_view MapHubNodeType(USB_HUB_NODE nodeType) noexcept
		{
			switch (nodeType) {
				case ::UsbHub:      return L"UsbHub";
				case ::UsbMIParent: return L"UsbMIParent";
				default:            return L"unknown";
			}
		}
	}

	HANDLE DeviceCommunication::GetFileHandle()
	{
		return _hFile.get();
	}

	DeviceCommunication::DeviceCommunication(std::wstring devicePath)
	{
		auto fileHandle = wil::unique_hfile(CreateFileW(
			devicePath.c_str(),
			GENERIC_WRITE,
			FILE_SHARE_WRITE,
			nullptr,
			OPEN_EXISTING,
			0,
			nullptr));

		if (!fileHandle || fileHandle.get() == INVALID_HANDLE_VALUE) {
			THROW_IF_WIN32_ERROR_MSG(HRESULT_FROM_WIN32(GetLastError()),
				"Failed to open device: DeviceCommunication constructor");
		}

		_hFile = std::move(fileHandle);
	}

	void DeviceCommunication::GetUsbHubNodeInformation(HubNodeInfo& nodeInfo)
	{
		USB_NODE_INFORMATION hubInfo{};
		ULONG bytesReturned = 0;

		BOOL ioctlResult = DeviceIoControl(
			_hFile.get(),
			IOCTL_USB_GET_NODE_INFORMATION,
			&hubInfo, sizeof(hubInfo),
			&hubInfo, sizeof(hubInfo),
			&bytesReturned,
			nullptr);

		THROW_LAST_ERROR_IF_MSG(!ioctlResult, "GetUsbHubNodeInformation: IOCTL_USB_GET_NODE_INFORMATION failed");

		nodeInfo.numbersOfPorts = hubInfo.u.HubInformation.HubDescriptor.bNumberOfPorts;
		nodeInfo.type = std::wstring(MapHubNodeType(hubInfo.NodeType));
	}

	void DeviceCommunication::GetUsbHubNodeInformationEx(HubNodeInfoEx& nodeInfo)
	{
		USB_HUB_INFORMATION_EX hubInfoEx{};
		ULONG bytesReturned = 0;

		BOOL ioctlResult = DeviceIoControl(
			_hFile.get(),
			IOCTL_USB_GET_HUB_INFORMATION_EX,
			&hubInfoEx, sizeof(hubInfoEx),
			&hubInfoEx, sizeof(hubInfoEx),
			&bytesReturned,
			nullptr);

		// This IOCTL may not be supported on older Windows versions (pre-Windows 8)
		bool isSupported = ioctlResult && (bytesReturned >= sizeof(USB_HUB_INFORMATION_EX));

		nodeInfo._isHubInfoExSupport = isSupported;
		nodeInfo._highestPortNumber = isSupported ? hubInfoEx.HighestPortNumber : 0;
	}

	void DeviceCommunication::GetUsbHubNodeCapabilitiesEx(HubNodeCapabilitiesEx& nodeInfo)
	{
		USB_HUB_CAPABILITIES_EX hubCapabilityEx{};
		ULONG bytesReturned = 0;

		BOOL ioctlResult = DeviceIoControl(
			_hFile.get(),
			IOCTL_USB_GET_HUB_CAPABILITIES_EX,
			&hubCapabilityEx, sizeof(hubCapabilityEx),
			&hubCapabilityEx, sizeof(hubCapabilityEx),
			&bytesReturned,
			nullptr);

		if (!ioctlResult || bytesReturned < sizeof(USB_HUB_CAPABILITIES_EX)) {
			THROW_HR_MSG(HRESULT_FROM_WIN32(GetLastError()),
				"GetUsbHubNodeCapabilitiesEx: IOCTL not supported on this OS version");
		}

		nodeInfo._hubIsRoot = hubCapabilityEx.CapabilityFlags.HubIsRoot != 0;
	}

	void DeviceCommunication::EnumeratePorts(ULONG numberOfPorts, std::map<size_t, HubPortInfo>& portConnectorProps)
	{
		if (numberOfPorts == 0) {
			throw InvalidDeviceArgumentException("numberOfPorts must be greater than 0");
		}
		if (numberOfPorts > UsbLimits::MaxPortsPerHub) {
			throw InvalidDeviceArgumentException("numberOfPorts exceeds maximum allowed value");
		}
		if (!_hFile || _hFile.get() == INVALID_HANDLE_VALUE) {
			throw InvalidDeviceHandleException("Device handle is invalid or not opened");
		}

		// Helper lambda to query port connector properties
		auto queryPortConnectorProps = [this](ULONG portIndex) -> std::optional<HubPortInfo>
		{
			// Phase 1: Query to get actual buffer size needed
			USB_PORT_CONNECTOR_PROPERTIES initialQuery{};
			initialQuery.ConnectionIndex = portIndex;
			ULONG bytesReturned = 0;

			BOOL result = DeviceIoControl(
				_hFile.get(),
				IOCTL_USB_GET_PORT_CONNECTOR_PROPERTIES,
				&initialQuery, sizeof(initialQuery),
				&initialQuery, sizeof(initialQuery),
				&bytesReturned,
				nullptr);

			if (!result || bytesReturned != sizeof(USB_PORT_CONNECTOR_PROPERTIES)) {
				return std::nullopt;
			}

			// Phase 2: Allocate full buffer and retrieve complete data
			auto propsBuffer = wil::make_unique_nothrow<BYTE[]>(initialQuery.ActualLength);
			if (!propsBuffer) {
				return std::nullopt;
			}

			auto& fullProps = *reinterpret_cast<PUSB_PORT_CONNECTOR_PROPERTIES>(propsBuffer.get());
			fullProps.ConnectionIndex = portIndex;

			result = DeviceIoControl(
				_hFile.get(),
				IOCTL_USB_GET_PORT_CONNECTOR_PROPERTIES,
				propsBuffer.get(), initialQuery.ActualLength,
				propsBuffer.get(), initialQuery.ActualLength,
				&bytesReturned,
				nullptr);

			HubPortInfo portInfo;
			if (!result || bytesReturned < initialQuery.ActualLength) {
				portInfo._isFilled = false;
			} else {
				portInfo._companionHubSymbolicLinkName = fullProps.CompanionHubSymbolicLinkName;
				portInfo._companionIndex = fullProps.CompanionIndex;
				portInfo._companionPortNumber = fullProps.CompanionPortNumber;
				portInfo._connectionIndex = fullProps.ConnectionIndex;
				portInfo._isFilled = true;
			}

			return portInfo;
		};

		// Enumerate all ports
		for (ULONG portNumber = 1; portNumber <= numberOfPorts; ++portNumber) {
			if (auto portInfo = queryPortConnectorProps(portNumber)) {
				portConnectorProps.try_emplace(portNumber, std::move(*portInfo));
			}
		}
	}

	void DeviceCommunication::EnumeratePortsConnectionInfo(ULONG numberOfPorts,
		std::map<size_t, HubConnectionInfo>& hubConnectionInfoList)
	{
		if (numberOfPorts == 0) {
			throw InvalidDeviceArgumentException("numberOfPorts must be greater than 0");
		}
		if (numberOfPorts > UsbLimits::MaxPortsPerHub) {
			throw InvalidDeviceArgumentException("numberOfPorts exceeds maximum allowed value");
		}
		if (!_hFile || _hFile.get() == INVALID_HANDLE_VALUE) {
			throw InvalidDeviceHandleException("Device handle is invalid or not opened");
		}

		hubConnectionInfoList.clear();

		// Helper: Query USB 3.0+ connection info (V2)
		auto queryConnectionInfoV2 = [this](size_t portIndex) -> std::optional<USB_NODE_CONNECTION_INFORMATION_EX_V2>
		{
			USB_NODE_CONNECTION_INFORMATION_EX_V2 infoV2{};
			infoV2.ConnectionIndex = static_cast<ULONG>(portIndex);
			infoV2.Length = sizeof(USB_NODE_CONNECTION_INFORMATION_EX_V2);
			infoV2.SupportedUsbProtocols.Usb300 = 1;

			ULONG bytesReturned = 0;
			BOOL result = DeviceIoControl(
				_hFile.get(),
				IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX_V2,
				&infoV2, sizeof(infoV2),
				&infoV2, sizeof(infoV2),
				&bytesReturned,
				nullptr);

			if (result && bytesReturned >= sizeof(USB_NODE_CONNECTION_INFORMATION_EX_V2)) {
				return infoV2;
			}
			return std::nullopt;
		};

		// Helper: Populate HubConnectionInfo from connection data
		auto populateConnectionInfo = [](HubConnectionInfo& info,
			const USB_NODE_CONNECTION_INFORMATION_EX& connEx,
			size_t portIndex,
			const std::optional<USB_NODE_CONNECTION_INFORMATION_EX_V2>& v2Info)
		{
			info._connectionIndex = portIndex;
			info._connectionStatus = connEx.ConnectionStatus;
			info._currentConfigurationValue = connEx.CurrentConfigurationValue;
			info._deviceAddress = connEx.DeviceAddress;
			info._deviceDescriptor = connEx.DeviceDescriptor;
			info._deviceIsHub = connEx.DeviceIsHub;
			info._numberOfOpenPipes = connEx.NumberOfOpenPipes;

			// Adjust speed for SuperSpeed devices reported as HighSpeed
			auto adjustedSpeed = connEx.Speed;
			if (connEx.Speed == UsbHighSpeed && v2Info.has_value()) {
				if (v2Info->Flags.DeviceIsOperatingAtSuperSpeedOrHigher ||
					v2Info->Flags.DeviceIsOperatingAtSuperSpeedPlusOrHigher) {
					adjustedSpeed = UsbSuperSpeed;
				}
			}
			info._speed = adjustedSpeed;

			// Copy pipe information
			info._pipeList.clear();
			info._pipeList.reserve(connEx.NumberOfOpenPipes);
			for (ULONG i = 0; i < connEx.NumberOfOpenPipes; ++i) {
				info._pipeList.push_back(connEx.PipeList[i]);
			}
		};

		// Helper: Query connection info using legacy IOCTL
		auto queryLegacyConnectionInfo = [this](size_t portIndex)
			-> std::optional<std::pair<HubConnectionInfo, bool>>
		{
			constexpr ULONG bufferSize = sizeof(USB_NODE_CONNECTION_INFORMATION) +
				sizeof(USB_PIPE_INFO) * UsbLimits::MaxEndpointsPerDevice;

			auto buffer = wil::make_unique_nothrow<BYTE[]>(bufferSize);
			THROW_IF_NULL_ALLOC(buffer);

			auto& connInfo = *reinterpret_cast<PUSB_NODE_CONNECTION_INFORMATION>(buffer.get());
			connInfo.ConnectionIndex = static_cast<ULONG>(portIndex);

			ULONG bytesReturned = bufferSize;
			BOOL result = DeviceIoControl(
				_hFile.get(),
				IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
				buffer.get(), bufferSize,
				buffer.get(), bufferSize,
				&bytesReturned,
				nullptr);

			if (!result) {
				return std::nullopt;
			}

			HubConnectionInfo info;
			info._connectionIndex = portIndex;
			info._connectionStatus = connInfo.ConnectionStatus;
			info._currentConfigurationValue = connInfo.CurrentConfigurationValue;
			info._deviceAddress = connInfo.DeviceAddress;
			info._deviceDescriptor = connInfo.DeviceDescriptor;
			info._deviceIsHub = connInfo.DeviceIsHub;
			info._numberOfOpenPipes = connInfo.NumberOfOpenPipes;
			info._speed = connInfo.LowSpeed ? UsbLowSpeed : UsbFullSpeed;

			info._pipeList.reserve(connInfo.NumberOfOpenPipes);
			for (ULONG i = 0; i < connInfo.NumberOfOpenPipes; ++i) {
				info._pipeList.push_back(connInfo.PipeList[i]);
			}

			bool isConnected = (connInfo.ConnectionStatus != NoDeviceConnected);
			return std::make_pair(std::move(info), isConnected);
		};

		// Process each port
		for (size_t portNumber = 1; portNumber <= numberOfPorts; ++portNumber) {
			// Try V2 query first (for USB 3.0+ speed detection)
			auto v2Info = queryConnectionInfoV2(portNumber);

			// Query extended connection info
			constexpr ULONG exBufferSize = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) +
				sizeof(USB_PIPE_INFO) * UsbLimits::MaxEndpointsPerDevice;

			auto exBuffer = wil::make_unique_nothrow<BYTE[]>(exBufferSize);
			THROW_IF_NULL_ALLOC(exBuffer);

			auto& connInfoEx = *reinterpret_cast<PUSB_NODE_CONNECTION_INFORMATION_EX>(exBuffer.get());
			connInfoEx.ConnectionIndex = static_cast<ULONG>(portNumber);

			ULONG bytesReturned = exBufferSize;
			BOOL exResult = DeviceIoControl(
				_hFile.get(),
				IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
				exBuffer.get(), exBufferSize,
				exBuffer.get(), exBufferSize,
				&bytesReturned,
				nullptr);

			HubConnectionInfo connectionInfo;
			bool deviceConnected = false;

			if (exResult) {
				populateConnectionInfo(connectionInfo, connInfoEx, portNumber, v2Info);
				deviceConnected = (connInfoEx.ConnectionStatus != NoDeviceConnected);
			} else {
				// Fall back to legacy IOCTL
				auto legacyResult = queryLegacyConnectionInfo(portNumber);
				if (!legacyResult) {
					continue;  // Skip this port
				}
				connectionInfo = std::move(legacyResult->first);
				deviceConnected = legacyResult->second;
			}

			// Retrieve driver key name for connected devices
			if (deviceConnected) {
				try {
					connectionInfo._driverKeyName = GetDriverKeyName(static_cast<ULONG>(portNumber));
				} catch (...) {
					// Driver key retrieval is non-critical
				}
			}

			hubConnectionInfoList.try_emplace(portNumber, std::move(connectionInfo));
		}
	}

	std::wstring DeviceCommunication::GetDriverKeyName(ULONG connectionIndex)
	{
		if (connectionIndex == 0) {
			throw InvalidDeviceArgumentException("connectionIndex must be greater than 0");
		}
		if (!_hFile || _hFile.get() == INVALID_HANDLE_VALUE) {
			throw InvalidDeviceHandleException("Device handle is invalid or not opened");
		}

		// Phase 1: Query to determine required buffer size
		USB_NODE_CONNECTION_DRIVERKEY_NAME initialQuery{};
		initialQuery.ConnectionIndex = connectionIndex;
		ULONG bytesReturned = 0;

		BOOL result = DeviceIoControl(
			_hFile.get(),
			IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
			&initialQuery, sizeof(initialQuery),
			&initialQuery, sizeof(initialQuery),
			&bytesReturned,
			nullptr);

		THROW_LAST_ERROR_IF_MSG(!result, "GetDriverKeyName: initial query failed");

		ULONG requiredSize = initialQuery.ActualLength;
		if (requiredSize <= sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME)) {
			THROW_HR_MSG(E_UNEXPECTED, "GetDriverKeyName: ActualLength too small");
		}

		// Phase 2: Allocate buffer and retrieve full driver key name
		auto keyNameBuffer = wil::make_unique_nothrow<BYTE[]>(requiredSize);
		THROW_IF_NULL_ALLOC(keyNameBuffer);

		auto& driverKeyName = *reinterpret_cast<PUSB_NODE_CONNECTION_DRIVERKEY_NAME>(keyNameBuffer.get());
		driverKeyName.ConnectionIndex = connectionIndex;

		result = DeviceIoControl(
			_hFile.get(),
			IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
			keyNameBuffer.get(), requiredSize,
			keyNameBuffer.get(), requiredSize,
			&bytesReturned,
			nullptr);

		THROW_LAST_ERROR_IF_MSG(!result, "GetDriverKeyName: retrieval failed");

		return std::wstring(driverKeyName.DriverKeyName);
	}

	PUSB_DESCRIPTOR_REQUEST DeviceCommunication::GetConfigDescriptor(ULONG connectionIndex,
		UCHAR descriptorIndex)
	{
		if (connectionIndex == 0) {
			throw InvalidDeviceArgumentException("connectionIndex must be greater than 0");
		}
		if (!_hFile || _hFile.get() == INVALID_HANDLE_VALUE) {
			throw InvalidDeviceHandleException("Device handle is invalid or not opened");
		}

		// Helper lambda to build and execute the descriptor request
		auto executeDescriptorRequest = [this, connectionIndex, descriptorIndex](
			BYTE* buffer, ULONG bufferSize) -> std::pair<bool, ULONG>
		{
			auto* request = reinterpret_cast<PUSB_DESCRIPTOR_REQUEST>(buffer);

			// Initialize request structure
			SecureZeroMemory(request, bufferSize);
			request->ConnectionIndex = connectionIndex;

			// Build setup packet for configuration descriptor request
			// High byte: descriptor type, Low byte: descriptor index
			request->SetupPacket.wValue =
				static_cast<USHORT>((USB_CONFIGURATION_DESCRIPTOR_TYPE << 8) | descriptorIndex);
			request->SetupPacket.wLength =
				static_cast<USHORT>(bufferSize - sizeof(USB_DESCRIPTOR_REQUEST));

			ULONG bytesReturned = 0;
			BOOL result = DeviceIoControl(
				_hFile.get(),
				IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
				buffer, bufferSize,
				buffer, bufferSize,
				&bytesReturned,
				nullptr);

			return { result != FALSE, bytesReturned };
		};

		// Phase 1: Query the descriptor to determine total configuration length
		constexpr ULONG initialBufferSize = sizeof(USB_DESCRIPTOR_REQUEST) + sizeof(USB_CONFIGURATION_DESCRIPTOR);
		BYTE queryBuffer[initialBufferSize];

		auto [querySuccess, queryBytesReturned] = executeDescriptorRequest(queryBuffer, initialBufferSize);

		if (!querySuccess || queryBytesReturned != initialBufferSize) {
			return nullptr;
		}

		// Extract wTotalLength from the returned configuration descriptor
		auto* configHeader = reinterpret_cast<PUSB_CONFIGURATION_DESCRIPTOR>(
			queryBuffer + sizeof(USB_DESCRIPTOR_REQUEST));

		USHORT totalDescriptorLength = configHeader->wTotalLength;
		if (totalDescriptorLength < sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
			return nullptr;
		}

		// Phase 2: Allocate appropriately sized buffer and retrieve complete descriptor
		ULONG fullBufferSize = sizeof(USB_DESCRIPTOR_REQUEST) + totalDescriptorLength;
		auto resultBuffer = wil::make_unique_nothrow<BYTE[]>(fullBufferSize);
		THROW_IF_NULL_ALLOC(resultBuffer);

		auto [fetchSuccess, fetchBytesReturned] = executeDescriptorRequest(resultBuffer.get(), fullBufferSize);

		if (!fetchSuccess || fetchBytesReturned != fullBufferSize) {
			return nullptr;
		}

		// Validate the complete descriptor
		auto* completeConfig = reinterpret_cast<PUSB_CONFIGURATION_DESCRIPTOR>(
			resultBuffer.get() + sizeof(USB_DESCRIPTOR_REQUEST));

		if (completeConfig->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR)) {
			return nullptr;
		}

		return reinterpret_cast<PUSB_DESCRIPTOR_REQUEST>(resultBuffer.release());
	}


	PSTRING_DESCRIPTOR_NODE DeviceCommunication::GetStringDescriptor(ULONG connectionIndex,
		UCHAR descriptorIndex,
		USHORT languageId
	)
	{
		if (connectionIndex == 0) {
			throw InvalidDeviceArgumentException("connectionIndex must be greater than 0");
		}
		if (!_hFile || _hFile.get() == INVALID_HANDLE_VALUE) {
			throw InvalidDeviceHandleException("Device handle is invalid or not opened");
		}

		// Prepare request buffer with space for maximum string length
		constexpr ULONG requestBufferSize = sizeof(USB_DESCRIPTOR_REQUEST) + MAXIMUM_USB_STRING_LENGTH;
		auto requestBuffer = std::make_unique<BYTE[]>(requestBufferSize);
		SecureZeroMemory(requestBuffer.get(), requestBufferSize);

		// Configure the descriptor request
		auto& request = *reinterpret_cast<PUSB_DESCRIPTOR_REQUEST>(requestBuffer.get());
		request.ConnectionIndex = connectionIndex;
		request.SetupPacket.wValue = static_cast<USHORT>((USB_STRING_DESCRIPTOR_TYPE << 8) | descriptorIndex);
		request.SetupPacket.wIndex = languageId;
		request.SetupPacket.wLength = static_cast<USHORT>(requestBufferSize - sizeof(USB_DESCRIPTOR_REQUEST));

		// Execute the IOCTL request
		ULONG bytesReturned = 0;
		BOOL ioctlResult = DeviceIoControl(
			_hFile.get(),
			IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
			requestBuffer.get(), requestBufferSize,
			requestBuffer.get(), requestBufferSize,
			&bytesReturned,
			nullptr);

		// Access the returned string descriptor (located after the request header)
		auto& stringDescriptor = *reinterpret_cast<PUSB_STRING_DESCRIPTOR>(requestBuffer.get() + sizeof(USB_DESCRIPTOR_REQUEST));

		// Validate the response using a structured approach
		auto validateResponse = [&]() -> bool {
			if (!ioctlResult) return false;
			if (bytesReturned < sizeof(USB_DESCRIPTOR_REQUEST) + 2) return false;
			if (stringDescriptor.bDescriptorType != USB_STRING_DESCRIPTOR_TYPE) return false;

			ULONG payloadSize = bytesReturned - sizeof(USB_DESCRIPTOR_REQUEST);
			if (stringDescriptor.bLength != payloadSize) return false;
			if (stringDescriptor.bLength % 2 != 0) return false;  // Unicode strings must be even length

			return true;
		};

		if (!validateResponse()) {
			spdlog::debug("GetStringDescriptor validation failed: Index={}, LangID=0x{:04X}, IOCTL={}, Bytes={}, Type=0x{:02X}, Len={}",
				descriptorIndex, languageId, ioctlResult ? 1 : 0, bytesReturned,
				bytesReturned > sizeof(USB_DESCRIPTOR_REQUEST) ? stringDescriptor.bDescriptorType : 0,
				bytesReturned > sizeof(USB_DESCRIPTOR_REQUEST) ? stringDescriptor.bLength : 0);
			return nullptr;
		}

		// Allocate and populate the result node
		ULONG nodeSize = sizeof(STRING_DESCRIPTOR_NODE) + stringDescriptor.bLength;
		auto resultNode = wil::make_unique_nothrow<BYTE[]>(nodeSize);
		THROW_IF_NULL_ALLOC(resultNode);

		auto* node = reinterpret_cast<PSTRING_DESCRIPTOR_NODE>(resultNode.get());
		SecureZeroMemory(node, nodeSize);

		node->DescriptorIndex = descriptorIndex;
		node->LanguageID = languageId;

		// Copy the descriptor data
		std::memcpy(node->StringDescriptor, &stringDescriptor, stringDescriptor.bLength);

		return reinterpret_cast<PSTRING_DESCRIPTOR_NODE>(resultNode.release());
	}


	void DeviceCommunication::GetUsbExternalHubName(DWORD index, std::wstring& hubName)
	{
		// Phase 1: Query to determine required buffer size
		USB_NODE_CONNECTION_NAME initialQuery{};
		initialQuery.ConnectionIndex = index;
		ULONG bytesReturned = 0;

		BOOL result = DeviceIoControl(
			_hFile.get(),
			IOCTL_USB_GET_NODE_CONNECTION_NAME,
			&initialQuery, sizeof(initialQuery),
			&initialQuery, sizeof(initialQuery),
			&bytesReturned,
			nullptr);

		if (!result) {
			spdlog::debug("GetUsbExternalHubName: initial query failed, error={}", GetLastError());
			THROW_LAST_ERROR_IF(true);
		}

		ULONG requiredSize = initialQuery.ActualLength;
		if (requiredSize <= sizeof(USB_NODE_CONNECTION_NAME)) {
			THROW_HR_MSG(E_UNEXPECTED, "GetUsbExternalHubName: ActualLength too small");
		}

		// Phase 2: Allocate buffer and retrieve full hub name
		auto hubNameBuffer = wil::make_unique_nothrow<BYTE[]>(requiredSize);
		THROW_IF_NULL_ALLOC(hubNameBuffer);

		auto& connectionName = *reinterpret_cast<PUSB_NODE_CONNECTION_NAME>(hubNameBuffer.get());
		connectionName.ConnectionIndex = index;

		result = DeviceIoControl(
			_hFile.get(),
			IOCTL_USB_GET_NODE_CONNECTION_NAME,
			hubNameBuffer.get(), requiredSize,
			hubNameBuffer.get(), requiredSize,
			&bytesReturned,
			nullptr);

		THROW_LAST_ERROR_IF(!result);

		hubName = connectionName.NodeName;
	}
}