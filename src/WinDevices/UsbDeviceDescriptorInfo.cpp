#include "pch.h"
#include "UsbDeviceDescriptorInfo.h"


void UsbDeviceDescriptorInfo::SetUsbDeviceInfo(std::wstring manufacturer,
	std::wstring product,
	std::wstring serialNumber)
{
	_manufacturer = manufacturer;
	_product = product;
	_serialNumber = serialNumber;
}


