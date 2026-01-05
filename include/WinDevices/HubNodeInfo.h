#pragma once

#include <string>
#include <Windows.h>

namespace KDM
{
	/// <summary>
	/// USB Hub node information from IOCTL_USB_GET_NODE_INFORMATION.
	/// Pure data structure with no invariants - uses struct per C.2.
	/// </summary>
	struct HubNodeInfo
	{
		// node type - https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/usbioctl/ne-usbioctl-_usb_hub_node
		std::wstring type;
		UCHAR numbersOfPorts = 0;
	};
}
