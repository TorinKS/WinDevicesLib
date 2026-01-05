#include "pch.h"
#include "DeviceProperty.h"

namespace KDM
{
	DeviceProperty::DeviceProperty(HDEVINFO deviceInfo, const SP_DEVINFO_DATA& deviceInfoData)
		: _deviceInfo{ deviceInfo }
		, _deviceInfoData{ deviceInfoData }
	{
	}

	DEVICE_POWER_STATE DeviceProperty::GetPowerState()
	{
		CM_POWER_DATA powerData{};

		BOOL querySuccess = SetupDiGetDeviceRegistryPropertyW(
			_deviceInfo,
			&_deviceInfoData,
			SPDRP_DEVICE_POWER_DATA,
			nullptr,
			reinterpret_cast<PBYTE>(&powerData),
			sizeof(powerData),
			nullptr);

		return querySuccess ? powerData.PD_MostRecentPowerState : PowerDeviceUnspecified;
	}

	bool DeviceProperty::GetStringProperty(DWORD property, std::wstring& resultValue)
	{
		// Two-phase query: first determine required buffer size, then retrieve data
		// Returns false instead of throwing since some properties may not exist for certain device types

		auto queryRequiredSize = [this, property]() -> std::optional<DWORD>
		{
			DWORD requiredSize = 0;

			SetupDiGetDeviceRegistryPropertyW(
				_deviceInfo,
				&_deviceInfoData,
				property,
				nullptr,
				nullptr,
				0,
				&requiredSize);

			DWORD errorCode = GetLastError();

			// Expected: ERROR_INSUFFICIENT_BUFFER with valid size
			if (requiredSize > 0 && errorCode == ERROR_INSUFFICIENT_BUFFER) {
				return requiredSize;
			}

			// Property doesn't exist or is inaccessible for this device type
			// Common for: Hardware Location on USB Root HUB, etc.
			if (errorCode == ERROR_INVALID_DATA || errorCode == ERROR_INVALID_REG_PROPERTY) {
				return std::nullopt;
			}

			// Unexpected error or zero size
			return std::nullopt;
		};

		auto sizeResult = queryRequiredSize();
		if (!sizeResult.has_value()) {
			return false;
		}

		DWORD bufferSize = sizeResult.value();
		auto buffer = std::make_unique<BYTE[]>(bufferSize);

		BOOL retrieveSuccess = SetupDiGetDeviceRegistryPropertyW(
			_deviceInfo,
			&_deviceInfoData,
			property,
			nullptr,
			buffer.get(),
			bufferSize,
			nullptr);

		if (!retrieveSuccess) {
			return false;
		}

		resultValue.assign(reinterpret_cast<const wchar_t*>(buffer.get()));
		return true;
	}
}
