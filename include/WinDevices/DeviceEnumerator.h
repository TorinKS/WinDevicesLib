#pragma once

#include "DevInfoData.h"
#include "IDeviceEnumerator.h"

namespace KDM
{
	class DevInfoData;

	/// @brief Enumerates device instances using Windows SetupAPI.
	///
	/// This class provides low-level device enumeration by interfacing with
	/// the Windows registry through SetupAPI functions (SetupDiGetClassDevs,
	/// SetupDiEnumDeviceInfo, etc.).
	///
	/// Common use cases:
	/// - Enumerate all USB devices: GUID_DEVINTERFACE_USB_DEVICE with DIGCF_PRESENT
	/// - Enumerate USB host controllers: GUID_CLASS_USB_HOST_CONTROLLER
	/// - Enumerate by device class: Pass specific class GUID with DIGCF_PRESENT
	///
	/// @note This class is move-only because HDEVINFO handles cannot be duplicated.
	/// The handle is automatically cleaned up via SetupDiDestroyDeviceInfoList.
	///
	/// @example
	/// @code
	/// DeviceEnumerator enumerator(GUID_DEVINTERFACE_USB_DEVICE,
	///     DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	/// auto devices = enumerator.GetDeviceInstances();
	/// for (const auto& device : devices) {
	///     // Process device.GetDriverKeyName(), device.GetHardwareId(), etc.
	/// }
	/// @endcode
	class DeviceEnumerator : public IDeviceEnumerator
	{
	public:
		/// @brief Constructs a device enumerator for devices matching the specified criteria.
		/// @param guid Device interface or setup class GUID to filter by.
		/// @param Options Flags controlling enumeration behavior (DIGCF_* values).
		///        Default: DIGCF_ALLCLASSES | DIGCF_PRESENT
		/// @throws wil::ResultException if SetupDiGetClassDevs fails.
		explicit DeviceEnumerator(const GUID& guid,
			DWORD Options = (DIGCF_ALLCLASSES | DIGCF_PRESENT));

		~DeviceEnumerator() = default;

		// Move-only semantics (HDEVINFO cannot be copied)
		DeviceEnumerator(const DeviceEnumerator&) = delete;
		DeviceEnumerator& operator=(const DeviceEnumerator&) = delete;
		DeviceEnumerator(DeviceEnumerator&&) noexcept = default;
		DeviceEnumerator& operator=(DeviceEnumerator&&) noexcept = default;

		/// @brief Returns the underlying HDEVINFO handle.
		/// @return The device information set handle.
		/// @note The returned handle is owned by this object; do not call
		///       SetupDiDestroyDeviceInfoList on it.
		[[nodiscard]] HDEVINFO GetDevInfoSet();

		/// @brief Enumerates all device instances matching the criteria.
		///
		/// Iterates through all matching devices using SetupDiEnumDeviceInfo
		/// and populates each DevInfoData with properties from the registry:
		/// - Driver key name (SPDRP_DRIVER)
		/// - Device description (SPDRP_DEVICEDESC)
		/// - Hardware ID (SPDRP_HARDWAREID)
		/// - Power state (SPDRP_DEVICE_POWER_DATA)
		///
		/// @return Vector of DevInfoData containing information about each device.
		/// @throws wil::ResultException if enumeration fails (other than ERROR_NO_MORE_ITEMS).
		[[nodiscard]] std::vector<DevInfoData> GetDeviceInstances() override;

	private:
		using unique_hdevinfo = wil::unique_any<HDEVINFO,
			decltype(&::SetupDiDestroyDeviceInfoList),
			::SetupDiDestroyDeviceInfoList>;

		unique_hdevinfo _hDevInfo;
	};
}
