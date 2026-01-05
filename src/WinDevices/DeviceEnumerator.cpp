#include "pch.h"
#include "DeviceEnumerator.h"
#include "DeviceProperty.h"
#include "UtilConvert.h"
#include "UsbClassCodes.h"
#include <spdlog/spdlog.h>
#include <utility>

namespace KDM
{
	DeviceEnumerator::DeviceEnumerator(const GUID& guid, DWORD options)
	{
		auto devInfoHandle = SetupDiGetClassDevsW(&guid, nullptr, nullptr, options);

		if (devInfoHandle == INVALID_HANDLE_VALUE) {
			THROW_HR_IF_MSG(HRESULT_FROM_WIN32(GetLastError()), true,
				"DeviceEnumerator: SetupDiGetClassDevs failed");
		}

		_hDevInfo.reset(devInfoHandle);
	}

	std::vector<DevInfoData> DeviceEnumerator::GetDeviceInstances()
	{
		std::vector<DevInfoData> deviceInstances;
		deviceInstances.reserve(UsbLimits::TypicalDeviceCount);

		// Helper: Populate device properties from registry
		auto populateDeviceProperties = [this](DevInfoData& deviceData, const SP_DEVINFO_DATA& spDevInfo)
		{
			DeviceProperty propReader(_hDevInfo.get(), spDevInfo);

			// Driver key name
			if (std::wstring driverKey; propReader.GetStringProperty(SPDRP_DRIVER, driverKey)) {
				deviceData.SetDriverKeyName(driverKey);
			}

			// Device description (e.g., "USB Mass Storage Device", "USB Composite Device")
			// Important for detecting USB Mass Storage devices that don't report via bDeviceClass
			if (std::wstring description; propReader.GetStringProperty(SPDRP_DEVICEDESC, description)) {
				deviceData.SetDeviceDescription(description);
			}

			// Hardware ID (e.g., USB\VID_0951&PID_172B)
			if (std::wstring hardwareId; propReader.GetStringProperty(SPDRP_HARDWAREID, hardwareId)) {
				deviceData.SetHardwareId(hardwareId);
			}

			// Power state
			deviceData.SetPowerState(propReader.GetPowerState());
		};

		// Enumerate all device instances
		for (ULONG deviceIndex = 0; ; ++deviceIndex) {
			SP_DEVINFO_DATA spDevInfoData{};
			spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

			BOOL enumResult = SetupDiEnumDeviceInfo(_hDevInfo.get(), deviceIndex, &spDevInfoData);

			if (!enumResult) {
				DWORD errorCode = GetLastError();
				if (errorCode == ERROR_NO_MORE_ITEMS) {
					break;  // Normal termination - no more devices
				}
				THROW_IF_WIN32_ERROR_MSG(errorCode, "DeviceEnumerator::GetDeviceInstances: SetupDiEnumDeviceInfo failed");
			}

			// Create device info and populate properties
			DevInfoData deviceData(_hDevInfo.get(), spDevInfoData);
			populateDeviceProperties(deviceData, spDevInfoData);

			deviceInstances.push_back(std::move(deviceData));
		}

		return deviceInstances;
	}

	HDEVINFO DeviceEnumerator::GetDevInfoSet()
	{
		return _hDevInfo.get();
	}
}