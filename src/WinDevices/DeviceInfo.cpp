#include "pch.h"
#include "DeviceInfo.h"
#include "DevInfoData.h"

namespace KDM
{
	DeviceInfo::DeviceInfo(const HDEVINFO DevInfo, const SP_DEVINFO_DATA DevInfoData) :
		_hDevInfo(DevInfo),
		_devInfoData(DevInfoData)
	{

	}

	DeviceInfo::DeviceInfo(const DevInfoData DevInfo)
	{

	}


	/// <summary>
	/// TODO: we can detect that device node has this interface ???
	/// </summary>
	/// <param name="hDevInfo"></param>
	/// <param name="devInfoData"></param>
	/// <param name="pGuid"></param>
	/// <returns></returns>
	bool DeviceInfo::HasInterface(const HDEVINFO hDevInfo, SP_DEVINFO_DATA devInfoData,
		LPGUID pGuid)
	{


		// HRESULT hr = CLSIDFromString(sGuid.c_str(), &guid);

		//THROW_HR_IF_MSG(hr,
		//	FAILED(hr), "DeviceInfo::HasInterface,  CLSIDFromString");


		SP_DEVICE_INTERFACE_DATA data;
		ZeroMemory(&data, sizeof(data));
		data.cbSize = sizeof(data);
		BOOL f = SetupDiEnumDeviceInterfaces(hDevInfo, &devInfoData, pGuid, 0, &data);

		return f;

	}

	SP_DEVICE_INTERFACE_DATA DeviceInfo::GetInterfaceDataByDevInfoData
	(
		HDEVINFO hDevInfo, SP_DEVINFO_DATA devInfoData,
		LPGUID pGuid
	)
	{

		// std::cout << "Get interfaces for: " << devInfoData.ClassGuid << std::endl;

		SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
		ZeroMemory(&deviceInterfaceData, sizeof(deviceInterfaceData));
		deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

		std::vector<SP_DEVICE_INTERFACE_DATA> deviceInterfacesData{};



		for (int i = 0; 
			SetupDiEnumDeviceInterfaces(hDevInfo, &devInfoData, pGuid, i, &deviceInterfaceData); 
			i++)
		{
			//if (!result)
			//{
			//	THROW_IF_WIN32_ERROR_MSG(GetLastError(), "DeviceInfo::GetInterfaceDataByDevInfoData, SetupDiEnumDeviceInterfaces");
			//}

			deviceInterfacesData.push_back(deviceInterfaceData);

			ZeroMemory(&deviceInterfaceData, sizeof(deviceInterfaceData));
			deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);


		}

		

		//std::cout << "SetupDiEnumDeviceInterfaces, " << result << " " <<  GetLastError() << std::endl;

		// TODO temporary return last interface data
		return deviceInterfacesData.at(0);
		//return deviceInterfaceData;

	}


	std::wstring DeviceInfo::GetDevicePathByInterfaceData(
		HDEVINFO hDevInfo,
		SP_DEVINFO_DATA devInfoData,
		SP_DEVICE_INTERFACE_DATA devInterfaceData,
		LPGUID pGuid

	)
	{

		PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData = nullptr;

		BOOL result;
		ULONG requiredSize = 0;
		HANDLE deviceHandle = nullptr;
		DWORD lastErrorCode;


		// figure out needed buffer size
		result = SetupDiGetDeviceInterfaceDetail(hDevInfo,
			&devInterfaceData,
			nullptr,
			0,
			&requiredSize,
			nullptr);

		lastErrorCode = GetLastError();
		if (!result)
		{
			if (lastErrorCode != ERROR_INSUFFICIENT_BUFFER)
			{
				THROW_IF_WIN32_ERROR_MSG(GetLastError(), "DeviceInfo::GetInterfaceDetailByInterfaceData, SetupDiGetDeviceInterfaceDetail");
			}
		}

		auto bufferData = std::make_unique< BYTE[] >(requiredSize);
		auto pDeviceDetailData = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(bufferData.get());
		pDeviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		result = SetupDiGetDeviceInterfaceDetail(hDevInfo,
			&devInterfaceData,
			pDeviceDetailData,
			requiredSize,
			&requiredSize,
			nullptr);

		if (!result)
		{
			THROW_IF_WIN32_ERROR_MSG(GetLastError(), "DeviceInfo::GetInterfaceDetailByInterfaceData, SetupDiGetDeviceInterfaceDetail");

		}

		return std::wstring(pDeviceDetailData->DevicePath);


	}

	std::wstring DeviceInfo::GetDeviceInstanceIdByDevInfo(HDEVINFO hDevInfo,
		SP_DEVINFO_DATA devInfoData)
	{
		DWORD requiredSize;


		auto status = SetupDiGetDeviceInstanceId(hDevInfo,
			&devInfoData,
			nullptr,
			0,
			&requiredSize
		);

		auto lastErrorCode = GetLastError();

		if (status != FALSE && lastErrorCode != ERROR_INSUFFICIENT_BUFFER)
		{
			THROW_HR(HRESULT_FROM_WIN32(lastErrorCode));
		}


		auto bufferPtr = std::make_unique<WCHAR[]>(requiredSize);
		auto pBuffer = bufferPtr.get();

		status = SetupDiGetDeviceInstanceId(hDevInfo,
			&devInfoData,
			pBuffer,
			requiredSize,
			&requiredSize
		);

		if (!status)
		{

			THROW_IF_WIN32_ERROR_MSG(HRESULT_FROM_WIN32(GetLastError()), " DeviceInfo::GetDeviceInstanceId(), SetupDiGetDeviceInstanceId");
		}

		return std::wstring{ pBuffer };
	}


	void DeviceInfo::PopulateInfo(LPGUID Guid)
	{
		_deviceInstanceId = GetDeviceInstanceIdByDevInfo(
			_hDevInfo, _devInfoData
		);

		_interfaceData = GetInterfaceDataByDevInfoData(
			_hDevInfo, _devInfoData,
			Guid
			//const_cast<LPGUID>(&GUID_DEVINTERFACE_USB_HOST_CONTROLLER)
		);

		_devicePath =
			GetDevicePathByInterfaceData(
				_hDevInfo, _devInfoData,
				_interfaceData,
				const_cast<LPGUID>(Guid)
			);


	}


	void DeviceInfo::PopulateUsbInfo()
	{
		// https://stackoverflow.com/questions/13927475/windows-how-to-enumerate-all-connected-usb-devices-device-path

		PopulateInfo(const_cast<LPGUID>(&GUID_DEVINTERFACE_USB_HUB));
		
	}

	/// <summary>
	/// stores all info retrieved by device instantce (SetupAPI) in its this class
	/// </summary>
	void DeviceInfo::PopulateUsbControllerInfo()
	{
		PopulateInfo(const_cast<LPGUID>(&GUID_DEVINTERFACE_USB_HOST_CONTROLLER));
	}

	std::wstring DeviceInfo::GetDevicePath()
	{
		return _devicePath;
	}

}