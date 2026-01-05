
#include "pch.h"
#include "DevInfoData.h"

namespace KDM
{

	DevInfoData::DevInfoData(const HDEVINFO DevInfo, const  SP_DEVINFO_DATA DevInfoData)
	{
		_devInfo = DevInfo;
		_devInfoData = DevInfoData;


		WCHAR tmpGuidBuffer[MAX_PATH] = { 0 };
		auto len = StringFromGUID2(_devInfoData.ClassGuid, tmpGuidBuffer, MAX_PATH);

		// THROW_LAST_ERROR_IF(0 == len);

		_classGuid = std::wstring(tmpGuidBuffer);
		// std::wcout << _classGuid << std::endl;
		// _classGuid

	}

	SP_DEVINFO_DATA DevInfoData::GetDevInfoData()
	{
		return _devInfoData;
	}

	void DevInfoData::SetDriverKeyName(const std::wstring& driverKeyName)
	{
		_driverKeyName = driverKeyName;
	}

	void DevInfoData::SetHardwareId(const std::wstring& hardwareId)
	{
		_hardwareId = hardwareId;
	}

	void DevInfoData::SetDeviceDescription(const std::wstring& value)
	{
		_deviceDescription = value;
	}

	std::wstring DevInfoData::GetDeviceDescription() const
	{
		return _deviceDescription;
	}


	void DevInfoData::SetPowerState(const DEVICE_POWER_STATE PowerState)
	{
		_powerState = PowerState;
	}

	std::wstring DevInfoData::GetPowerStateAsString()
	{
		switch (_powerState)
		{
		case 0:
			return L"PowerDeviceUnspecified";
		case 1:
			return L"PowerDeviceD0";
		case 2:
			return L"PowerDeviceD1";
		case 3:
			return L"PowerDeviceD2";
		case 4:
			return L"PowerDeviceD3";
		case 5:
			return L"PowerDeviceMaximum";
		default:
			return L"PowerDeviceUnspecified";

		}
	}

	DEVICE_POWER_STATE DevInfoData::GetPowerState()
	{
		return _powerState;
	}


}