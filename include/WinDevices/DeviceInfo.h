#pragma once

//#include "AbstractDevice.h"
namespace KDM
{
    class DevInfoData;

    class DeviceInfo
    {
    public:

        DeviceInfo(const HDEVINFO DevInfo, const SP_DEVINFO_DATA DevInfoData);
        DeviceInfo(const DevInfoData DevInfo);
        bool HasInterface(const HDEVINFO hDevInfo, SP_DEVINFO_DATA devInfoData, LPGUID pGuid);

        void PopulateUsbControllerInfo();
        void PopulateUsbInfo();

        SP_DEVICE_INTERFACE_DATA GetInterfaceDataByDevInfoData(
            HDEVINFO hDevInfo, SP_DEVINFO_DATA devInfoData,
            LPGUID pGuid
        );

        std::wstring GetDevicePathByInterfaceData(
            HDEVINFO hDevInfo,
            SP_DEVINFO_DATA devInfoData,
            SP_DEVICE_INTERFACE_DATA devInterfaceData,
            LPGUID pGuid

        );

        std::wstring GetDeviceInstanceIdByDevInfo(HDEVINFO hDevInfo,
            SP_DEVINFO_DATA devInfoData);

        std::wstring GetDevicePath();

        std::wstring GetDeviceInstanceId() { return _deviceInstanceId;  }



    private:
        
        void PopulateInfo(LPGUID Guid);
        
        HDEVINFO _hDevInfo = nullptr;
        SP_DEVINFO_DATA _devInfoData;
        SP_DEVICE_INTERFACE_DATA _interfaceData;
        std::wstring _devicePath;
        std::wstring _deviceInstanceId;


    };

}