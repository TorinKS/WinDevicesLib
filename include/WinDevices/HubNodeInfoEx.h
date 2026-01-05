#pragma once

#include <Windows.h>

namespace KDM
{
	/// <summary>
	/// Extended USB Hub information (Windows 8+).
	/// https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/usbioctl/ns-usbioctl-_usb_hub_information_ex
	/// Pure data structure with no invariants - uses struct per C.2.
	/// </summary>
	struct HubNodeInfoEx
	{
		USHORT _highestPortNumber = 0;
		// is IOCTL_USB_GET_HUB_INFORMATION_EX supported by OS?
		// Windows 7 doesn't support that
		bool _isHubInfoExSupport = false;
	};
}