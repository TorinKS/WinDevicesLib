#pragma once

#include <string>
#include <Windows.h>

namespace KDM
{
	/// <summary>
	/// USB Hub port properties container.
	/// Pure data structure with no invariants - uses struct per C.2.
	/// </summary>
	struct HubPortInfo
	{
		// one based port number
		ULONG _connectionIndex = 0;
		// bitmask of flags indicating properties and capabilities of the port
		// USB_PORT_PROPERTIES is unsupported on Win7 SDK
		//USB_PORT_PROPERTIES  _usbPortProperties;
		// Zero based index number of the companion port being queried.
		USHORT _companionIndex = 0;
		// Port number of the companion port
		USHORT _companionPortNumber = 0;
		// Symbolic link name for the companion hub
		std::wstring _companionHubSymbolicLinkName;

		// if == false this means that structure was not filled for the given port
		bool _isFilled = false;
	};
}
