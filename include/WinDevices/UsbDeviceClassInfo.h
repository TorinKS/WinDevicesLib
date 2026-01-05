#pragma once

namespace KDM
{
	class UsbDeviceClassInfo
	{
	public:
		UsbDeviceClassInfo();
		std::wstring GetBaseClassByCode(UCHAR code);

	private:

		void InitUsbBaseClasses();
		std::map<UCHAR, std::wstring > _usbBaseClassList;

	};
}