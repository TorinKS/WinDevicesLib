#pragma once

#include <string>
#include <vector>
#include <Windows.h>
#include <usb.h>
#include <usbioctl.h>

namespace KDM
{
	/// <summary>
	/// USB Hub port connection information.
	/// Pure data structure with no invariants - uses struct per C.2.
	/// </summary>
	struct HubConnectionInfo
	{
		ULONG _connectionIndex = 0;
		USB_DEVICE_DESCRIPTOR _deviceDescriptor = {};
		UCHAR _currentConfigurationValue = 0;
		UCHAR _speed = 0;
		BOOLEAN _deviceIsHub = FALSE;
		USHORT _deviceAddress = 0;
		ULONG _numberOfOpenPipes = 0;
		USB_CONNECTION_STATUS _connectionStatus = NoDeviceConnected;
		std::vector<USB_PIPE_INFO> _pipeList;

		std::wstring _driverKeyName;
	};
}
