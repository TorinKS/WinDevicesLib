#pragma once

#include <string>
#include <Windows.h>
#include <SetupAPI.h>

namespace KDM
{
	/// <summary>
	/// Wrapper around Windows SP_DEVINFO_DATA with additional device properties.
	/// </summary>
	class DevInfoData
	{
	public:
		// HDEVINFO DevInfo, SP_DEVINFO_DATA DevInfoData
		DevInfoData(const HDEVINFO DevInfo, const SP_DEVINFO_DATA DevInfoData);
		SP_DEVINFO_DATA GetDevInfoData();
		void SetDriverKeyName(const std::wstring& driverKeyName);
		void SetHardwareId(const std::wstring& hardwareId);
		void SetPowerState(const DEVICE_POWER_STATE PowerState);
		void SetDeviceDescription(const std::wstring& value);

		DEVICE_POWER_STATE GetPowerState();
		std::wstring GetPowerStateAsString();
		std::wstring GetDriverKeyName() const { return _driverKeyName; }
		std::wstring GetHardwareId() const { return _hardwareId; }
		std::wstring GetDeviceDescription() const;
		GUID GetClassGuid() const { return _devInfoData.ClassGuid; }

	private:
		HDEVINFO _devInfo = nullptr;
		SP_DEVINFO_DATA _devInfoData = {};
		std::wstring _classGuid;
		std::wstring _driverKeyName;
		std::wstring _hardwareId;
		DEVICE_POWER_STATE _powerState = PowerDeviceUnspecified;
		std::wstring _deviceDescription;
	};
}