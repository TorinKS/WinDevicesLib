
#include "pch.h"
#include "UtilConvert.h"
#include "UsbClassCodes.h"

namespace KDM
{
	/// <summary>
	/// Converts wide string to UTF-8 encoded string for logging and display
	/// </summary>
	/// <param name="wstr">Wide string to convert</param>
	/// <returns>UTF-8 encoded string</returns>
	std::string UtilConvert::WStringToUTF8(std::wstring_view wstr)
	{
		if (wstr.empty()) return "";
		int size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
		if (size == 0) return "";
		std::string result(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), &result[0], size, nullptr, nullptr);
		return result;
	}

	/// <summary>
	/// Converts UTF-8 encoded string to wide string
	/// </summary>
	/// <param name="str">UTF-8 encoded string to convert</param>
	/// <returns>Wide string</returns>
	std::wstring UtilConvert::UTF8ToWString(std::string_view str)
	{
		if (str.empty()) return L"";
		int size = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
		if (size == 0) return L"";
		std::wstring result(size, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &result[0], size);
		return result;
	}

	/// <summary>
	/// converts passed integer value to the corresponding hex string value
	/// </summary>
	/// <param name="Value">integer value which will be converted</param>
	/// <param name="BytesNumber">the width of retrieved hex value</param>
	/// <returns></returns>
	std::wstring UtilConvert::GetHexIdAsString(USHORT Value, USHORT BytesNumber)
	{
		WCHAR buff[16] = { 0 };

		std::wstring test = L"0x%0" + std::to_wstring(BytesNumber) + L"x";
		auto res = swprintf_s(buff, ARRAYSIZE(buff), test.c_str(), Value);


		if (res < 0)
		{
			THROW_HR(E_INVALIDARG);
		}

		return std::wstring(buff);


	}

	std::wstring UtilConvert::GetBaseClassById(USHORT value)
	{
		// USB-IF Class Codes
		// Reference: https://www.usb.org/defined-class-codes
		// Reference: https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/supported-usb-classes
		switch (value)
		{
		case UsbClass::InterfaceClassDefined:
			return L"USB interface class device";
		case UsbClass::Audio:
			return L"Audio";
		case UsbClass::CdcControl:
			return L"Communications and CDC Control";
		case UsbClass::Hid:
			return L"HID (Human Interface Device)";
		case UsbClass::Physical:
			return L"Physical";
		case UsbClass::Image:
			return L"Image";
		case UsbClass::Printer:
			return L"Printer";
		case UsbClass::MassStorage:
			return L"Mass Storage";
		case UsbClass::Hub:
			return L"Hub";
		case UsbClass::CdcData:
			return L"CDC-Data";
		case UsbClass::SmartCard:
			return L"Smart Card";
		case UsbClass::ContentSecurity:
			return L"Content Security";
		case UsbClass::Video:
			return L"Video";
		case UsbClass::PersonalHealthcare:
			return L"Personal Healthcare";
		case UsbClass::AudioVideo:
			return L"Audio/Video";
		case UsbClass::Billboard:
			return L"Billboard";
		case UsbClass::TypeCBridge:
			return L"USB Type-C Bridge";
		case UsbClass::BulkDisplay:
			return L"Bulk Display";
		case UsbClass::Mctp:
			return L"MCTP over USB";
		case UsbClass::I3C:
			return L"I3C";
		case UsbClass::Diagnostic:
			return L"Diagnostic Device";
		case UsbClass::WirelessController:
			return L"Wireless Controller";
		case UsbClass::Miscellaneous:
			return L"Miscellaneous";
		case UsbClass::ApplicationSpecific:
			return L"Application Specific";
		case UsbClass::VendorSpecific:
			return L"Vendor Specific";
		default:
			return L"Unknown";
		}
	}

	std::wstring UtilConvert::GetUsbClassNameByDescId(UCHAR DeviceClass)
	{
		return GetBaseClassById(DeviceClass);
	}
}