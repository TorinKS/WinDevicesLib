#include "pch.h"
#include "UsbDeviceClassInfo.h"

namespace KDM
{
	UsbDeviceClassInfo::UsbDeviceClassInfo()
	{
		InitUsbBaseClasses();
	}

	std::wstring UsbDeviceClassInfo::GetBaseClassByCode(UCHAR code)
	{
		if (_usbBaseClassList.find(code) != _usbBaseClassList.end())
		{
			return _usbBaseClassList[code];
		}
		else
		{
			return L"Unknown";
		}

	}

	void UsbDeviceClassInfo::InitUsbBaseClasses()
	{
		// https://www.usb.org/defined-class-codes#anchor_BaseClass01h
		_usbBaseClassList.emplace(0x0, L"Interface class device");
		_usbBaseClassList.emplace(0x1, L"Audio");
		_usbBaseClassList.emplace(0x2, L"Communications and CDC Control");
		_usbBaseClassList.emplace(0x3, L"HID (Human Interface Device)");
		_usbBaseClassList.emplace(0x5, L"Physical");
		_usbBaseClassList.emplace(0x6, L"Image");
		_usbBaseClassList.emplace(0x7, L"Printer");
		_usbBaseClassList.emplace(0x8, L"Mass Storage");
		_usbBaseClassList.emplace(0x9, L"Hub");
		_usbBaseClassList.emplace(0xA, L"CDC-Data");
		_usbBaseClassList.emplace(0xB, L"Smart Card");
		_usbBaseClassList.emplace(0xD, L"Content Security");
		_usbBaseClassList.emplace(0xE, L"Video");
		_usbBaseClassList.emplace(0xF, L"Personal Healthcare");
		_usbBaseClassList.emplace(0x10, L"Audio/Video Devices");
		_usbBaseClassList.emplace(0x11, L"Billboard Device Class");
		_usbBaseClassList.emplace(0x12, L"USB Type - C Bridge Class");
		_usbBaseClassList.emplace(0x13, L"USB Bulk Display Protocol Device Class");
		_usbBaseClassList.emplace(0x14, L"MCTP over USB Protocol Endpoint Device Class");
		_usbBaseClassList.emplace(0x3C, L"I3C Device Class");
		_usbBaseClassList.emplace(0xDC, L"Diagnostic Device");
		_usbBaseClassList.emplace(0xE0, L"Wireless Controller");
		_usbBaseClassList.emplace(0xEF, L"Miscellaneous");
		_usbBaseClassList.emplace(0xFE, L"Application Specific");
		_usbBaseClassList.emplace(0xFF, L"Vendor Specific");

	}
}