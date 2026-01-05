#pragma once

namespace KDM
{
	/// @brief Retrieves device properties from Windows registry via SetupAPI.
	///
	/// This class wraps SetupDiGetDeviceRegistryProperty to provide a convenient
	/// interface for querying device properties such as driver key, hardware ID,
	/// device description, and power state.
	///
	/// @note Some properties may not be available for certain device types.
	/// GetStringProperty returns false (rather than throwing) when a property
	/// doesn't exist, as this is expected behavior for some devices.
	///
	/// @example
	/// @code
	/// DeviceProperty propReader(hDevInfo, devInfoData);
	/// if (std::wstring driverKey; propReader.GetStringProperty(SPDRP_DRIVER, driverKey)) {
	///     // Use driverKey
	/// }
	/// @endcode
	class DeviceProperty
	{
	public:
		/// @brief Constructs a DeviceProperty reader for a specific device.
		/// @param deviceInfo Handle to a device information set (HDEVINFO).
		/// @param deviceInfoData Reference to SP_DEVINFO_DATA identifying the device.
		DeviceProperty(HDEVINFO deviceInfo, const SP_DEVINFO_DATA& deviceInfoData);

		/// @brief Retrieves a string property from the device registry.
		///
		/// Uses a two-phase query: first to determine required buffer size,
		/// then to retrieve the actual property value.
		///
		/// @param property The SPDRP_* property identifier (e.g., SPDRP_DRIVER, SPDRP_DEVICEDESC).
		/// @param[out] resultValue Receives the property value if successful.
		/// @return true if the property was retrieved successfully, false if the property
		///         doesn't exist or is inaccessible for this device type.
		///
		/// @note Returns false (not throw) for ERROR_INVALID_DATA and ERROR_INVALID_REG_PROPERTY,
		/// as these indicate the property simply doesn't exist for this device.
		bool GetStringProperty(DWORD property, std::wstring& resultValue);

		/// @brief Retrieves the current power state of the device.
		/// @return The device's power state, or PowerDeviceUnspecified if unavailable.
		DEVICE_POWER_STATE GetPowerState();

	private:
		HDEVINFO _deviceInfo;
		SP_DEVINFO_DATA _deviceInfoData;
	};
}
