#pragma once

#include <vector>

namespace KDM
{
	class DevInfoData;

	/// <summary>
	/// Interface for device enumeration operations.
	/// Enables dependency injection and mock implementations for unit testing.
	/// </summary>
	class IDeviceEnumerator
	{
	public:
		virtual ~IDeviceEnumerator() = default;

		// Prevent copying (interface should be used via pointer/reference)
		IDeviceEnumerator(const IDeviceEnumerator&) = delete;
		IDeviceEnumerator& operator=(const IDeviceEnumerator&) = delete;

		// Allow moving for derived classes
		IDeviceEnumerator(IDeviceEnumerator&&) noexcept = default;
		IDeviceEnumerator& operator=(IDeviceEnumerator&&) noexcept = default;

		virtual std::vector<DevInfoData> GetDeviceInstances() = 0;

	protected:
		// Protected default constructor - only derived classes can instantiate
		IDeviceEnumerator() = default;
	};
}

