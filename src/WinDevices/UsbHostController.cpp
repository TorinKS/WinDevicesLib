

#include "pch.h"
#include "DeviceCommunication.h"
#include "UsbHostController.h"

namespace KDM
{

	UsbHostController::UsbHostController(std::wstring devicePath,
		DeviceCommunication& deviceCommunication) :
		_devicePath(devicePath),
		_deviceCommunication(deviceCommunication)
	{


	}

	void UsbHostController::PopulateInfo()
	{
		USBUSER_CONTROLLER_INFO_0  usbControllerInfo = { 0 };
		DWORD dwError = 0;
		DWORD neededSize = 0;
		BOOL  result = FALSE;

		// USB_CONTROLLER_INFO_0 contains NumberOfRootPorts member which describets the number of ports

		usbControllerInfo.Header.UsbUserRequest = USBUSER_GET_CONTROLLER_INFO_0;
		// usbControllerInfo.Header.UsbUserRequest = RequestType;
		usbControllerInfo.Header.RequestBufferLength = sizeof(usbControllerInfo);

		result = DeviceIoControl(_deviceCommunication.GetFileHandle(),
			IOCTL_USB_USER_REQUEST,
			&usbControllerInfo,
			sizeof(usbControllerInfo),
			&usbControllerInfo,
			sizeof(usbControllerInfo),
			&neededSize,
			nullptr);

		THROW_HR_IF_MSG(HRESULT_FROM_WIN32(GetLastError()),
			!result, "UsbHostController::PopulateInfo, DeviceIoControl");


		// store info about number of ports
		_numberOfPorts = usbControllerInfo.Info0.NumberOfRootPorts;

		_pciVendorId = usbControllerInfo.Info0.PciVendorId;
		_pciDeviceId = usbControllerInfo.Info0.PciDeviceId;
		_pciRevision = usbControllerInfo.Info0.PciRevision;

		// get info about root hub name
		_rootHubName = GetRootHubNameByHandle(_deviceCommunication.GetFileHandle());
		_driverKeyName = GetDriverKeyName(_deviceCommunication.GetFileHandle());
	}


	std::wstring UsbHostController::GetRootHubNameByHandle(HANDLE HostController)
	{
		BOOL result = FALSE;
		USB_ROOT_HUB_NAME   rootHubName = { 0 };
		PUSB_ROOT_HUB_NAME  pRootHubName = nullptr;
		// PCHAR               rootHubNameA = NULL;
		DWORD requiredSize;
		//DWORD lastErrorCode;


		result = DeviceIoControl(HostController,
			IOCTL_USB_GET_ROOT_HUB_NAME,
			0,
			0,
			&rootHubName,
			sizeof(rootHubName),
			&requiredSize,
			nullptr);


		THROW_HR_IF_MSG(HRESULT_FROM_WIN32(GetLastError()),
			!result, "UsbHostController::GetRootHubName, DeviceIoControl");


		requiredSize = rootHubName.ActualLength;

		auto bufferPtr = std::make_unique < BYTE[] >(requiredSize);
		auto pBuffer = bufferPtr.get();

		result = DeviceIoControl(HostController,
			IOCTL_USB_GET_ROOT_HUB_NAME,
			nullptr,
			0,
			pBuffer,
			requiredSize,
			&requiredSize,
			nullptr);

		THROW_HR_IF_MSG(HRESULT_FROM_WIN32(GetLastError()),
			!result, "UsbHostController::GetRootHubName, DeviceIoControl");



		pRootHubName = reinterpret_cast<PUSB_ROOT_HUB_NAME>(pBuffer);

		// return something like
		// USB#ROOT_HUB30#4&380cfd04&0&0#{f18a0e88-c30c-11d0-8815-00a0c906bed8}
		return std::wstring(pRootHubName->RootHubName);

	}

	std::wstring UsbHostController::GetDriverKeyName(HANDLE hFile)
	{
		USB_HCD_DRIVERKEY_NAME  driverKeyName = { 0 };
		PUSB_HCD_DRIVERKEY_NAME pDriverKeyName = nullptr;
		ULONG neededSize = 0;
		BOOL result;



		ZeroMemory(&driverKeyName, sizeof(driverKeyName));

		result = DeviceIoControl(hFile,
			IOCTL_GET_HCD_DRIVERKEY_NAME,
			&driverKeyName,
			sizeof(driverKeyName),
			&driverKeyName,
			sizeof(driverKeyName),
			&neededSize,
			nullptr);

		THROW_HR_IF_MSG(HRESULT_FROM_WIN32(GetLastError()),
			!result, "UsbHostController::GetDriverKeyName, DeviceIoControl");




		neededSize = driverKeyName.ActualLength;

		// something wrong with kernel mode code?
		// TODO: replace with correct error code
		THROW_HR_IF(E_INVALIDARG, (neededSize <= sizeof(driverKeyName)));

		auto driverKeyNamePtr = std::make_unique < BYTE[]>(neededSize);
		pDriverKeyName = reinterpret_cast<PUSB_HCD_DRIVERKEY_NAME>(driverKeyNamePtr.get());


		result = DeviceIoControl(hFile,
			IOCTL_GET_HCD_DRIVERKEY_NAME,
			pDriverKeyName,
			neededSize,
			pDriverKeyName,
			neededSize,
			&neededSize,
			nullptr);

		THROW_HR_IF_MSG(HRESULT_FROM_WIN32(GetLastError()),
			!result, "UsbHostController::GetDriverKeyName, DeviceIoControl");


		std::wstring resultString(pDriverKeyName->DriverKeyName);
		return resultString;

	}

	std::wstring UsbHostController::GetRootHubName()
	{
		return _rootHubName;
	}

}