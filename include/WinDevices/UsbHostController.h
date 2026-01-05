#pragma once

#include <string>
#include <Windows.h>

namespace KDM
{
	class DeviceCommunication;

	/// <summary>
	/// Represents a USB Host Controller device.
	/// This class is non-copyable and non-movable due to reference member.
	/// </summary>
	class UsbHostController
	{
	public:
		UsbHostController(std::wstring devicePath, DeviceCommunication& deviceCommunication);
		~UsbHostController() = default;

		// Rule of Five: Non-copyable, non-movable (contains reference member)
		UsbHostController(const UsbHostController&) = delete;
		UsbHostController& operator=(const UsbHostController&) = delete;
		UsbHostController(UsbHostController&&) = delete;
		UsbHostController& operator=(UsbHostController&&) = delete;

		std::wstring GetDriverKeyName(HANDLE hFile);
		std::wstring GetRootHubNameByHandle(HANDLE HostController);
		std::wstring GetRootHubName();
		void PopulateInfo();

		std::wstring _devicePath;
		std::wstring _rootHubName;
		std::wstring _driverKeyName;
		DeviceCommunication& _deviceCommunication;
		ULONG _numberOfPorts = 0;
		ULONG _pciVendorId = 0;
		ULONG _pciDeviceId = 0;
		ULONG _pciRevision = 0;
	};
}