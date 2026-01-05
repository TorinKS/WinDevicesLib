#pragma once

namespace KDM
{
	/// <summary>
	/// Extended USB Hub capabilities.
	/// Pure data structure with no invariants - uses struct per C.2.
	/// </summary>
	struct HubNodeCapabilitiesEx
	{
		bool _hubIsRoot = false;
	};
}