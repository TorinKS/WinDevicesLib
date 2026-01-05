#include "pch.h"
#include "DeviceResultantInfo.h"
#include "DevInfoData.h"
#include "DevicesManager.h"
#include "DeviceProperty.h"
#include "DeviceInfo.h"
#include "DeviceEnumerator.h"
#include "DeviceCommunication.h"
#include "UsbHub.h"
#include "UsbHostController.h"
#include "UtilConvert.h"
#include "UsbVendorList.h"
#include "UsbDeviceClassInfo.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace KDM
{

namespace
{
	// Helper to format GUID as string for logging
	std::wstring FormatGuid(const GUID& guid)
	{
		wchar_t buffer[40]{};
		StringFromGUID2(guid, buffer, 40);
		return buffer;
	}

	// Helper to build VID/PID search pattern
	std::wstring BuildVidPidPattern(USHORT vendorId, USHORT productId)
	{
		wchar_t buffer[32]{};
		swprintf_s(buffer, L"VID_%04X&PID_%04X", vendorId, productId);
		return buffer;
	}

	// Case-insensitive wstring search
	bool ContainsIgnoreCase(const std::wstring& haystack, const std::wstring& needle)
	{
		if (needle.empty()) return true;
		if (haystack.empty()) return false;

		std::wstring haystackUpper = haystack;
		std::wstring needleUpper = needle;
		std::transform(haystackUpper.begin(), haystackUpper.end(), haystackUpper.begin(), ::towupper);
		std::transform(needleUpper.begin(), needleUpper.end(), needleUpper.begin(), ::towupper);

		return haystackUpper.find(needleUpper) != std::wstring::npos;
	}
}

// PIMPL Implementation Class
class DevicesManager::Impl
{
public:
	Impl() = default;
	~Impl() = default;

	Impl(const Impl&) = delete;
	Impl& operator=(const Impl&) = delete;
	Impl(Impl&&) noexcept = default;
	Impl& operator=(Impl&&) noexcept = default;

	void EnumerateUsbDevices();
	void EnumerateByDeviceClass(const GUID& deviceClassGuid);

	void AddDeviceInfo(DeviceResultantInfo deviceResultantInfo)
	{
		_devicesList.push_back(std::move(deviceResultantInfo));
		spdlog::trace("AddDeviceInfo: Device added (total: {})", _devicesList.size());
	}

	[[nodiscard]] const std::vector<DeviceResultantInfo>& GetDevices() const noexcept
	{
		return _devicesList;
	}

	[[nodiscard]] size_t GetDeviceCount() const noexcept
	{
		return _devicesList.size();
	}

	void ClearDevices() noexcept
	{
		_devicesList.clear();
	}

private:
	void EnumeratePortsFromRootHub(const std::wstring& hubName,
		const std::vector<DevInfoData>& allDevices,
		HDEVINFO devInfoSet);

	std::vector<DeviceResultantInfo> _devicesList;
};

void DevicesManager::Impl::EnumeratePortsFromRootHub(const std::wstring& hubName,
	const std::vector<DevInfoData>& allDevices,
	HDEVINFO devInfoSet)
{
	spdlog::info("EnumeratePortsFromRootHub: Starting for hub: {}", UtilConvert::WStringToUTF8(hubName));

	UsbHub usbHub(hubName);
	usbHub.PopulateInfo();

	spdlog::debug("EnumeratePortsFromRootHub: Hub info populated");

	const auto& portConnectionInfo = usbHub.GetPortConnectionInfo();

	// Port metadata maps
	std::map<size_t, UCHAR> deviceClassMap;
	std::map<size_t, USHORT> vendorIdMap;
	std::map<size_t, USHORT> productIdMap;
	std::map<size_t, GUID> setupClassGuidMap;

	// Process connected devices on each port
	for (const auto& [portNumber, connectionInfo] : portConnectionInfo)
	{
		if (connectionInfo._connectionStatus == NoDeviceConnected) {
			continue;
		}

		const auto& descriptor = connectionInfo._deviceDescriptor;
		spdlog::info("Port {}: Connected device found", portNumber);
		spdlog::info("  idProduct: {}", UtilConvert::WStringToUTF8(UtilConvert::GetHexIdAsString(descriptor.idProduct, 4)));
		spdlog::info("  idVendor: {}", UtilConvert::WStringToUTF8(UtilConvert::GetHexIdAsString(descriptor.idVendor, 4)));
		spdlog::info("  bDeviceClass: {} (0x{:02X})",
			UtilConvert::WStringToUTF8(UtilConvert::GetUsbClassNameByDescId(descriptor.bDeviceClass)),
			descriptor.bDeviceClass);
		spdlog::info("  DriverKeyName: {}", UtilConvert::WStringToUTF8(connectionInfo._driverKeyName));
		spdlog::info("  IsHub: {}", connectionInfo._deviceIsHub);

		// Store device metadata
		deviceClassMap.try_emplace(portNumber, descriptor.bDeviceClass);
		vendorIdMap.try_emplace(portNumber, descriptor.idVendor);
		productIdMap.try_emplace(portNumber, descriptor.idProduct);

		std::wstring vidPidPattern = BuildVidPidPattern(descriptor.idVendor, descriptor.idProduct);
		spdlog::info("  Searching for pattern: {}", UtilConvert::WStringToUTF8(vidPidPattern));

		// Find matching devices and collect their GUIDs
		std::vector<GUID> matchedGuids;
		std::optional<DevInfoData> usbBusLayerDevice;

		for (const auto& device : allDevices)
		{
			std::wstring hardwareId = device.GetHardwareId();
			if (hardwareId.empty() || hardwareId.find(vidPidPattern) == std::wstring::npos) {
				continue;
			}

			GUID classGuid = device.GetClassGuid();
			spdlog::info("  MATCHED: HwID={}, ClassGuid={}",
				UtilConvert::WStringToUTF8(hardwareId),
				UtilConvert::WStringToUTF8(FormatGuid(classGuid)));

			matchedGuids.push_back(classGuid);

			// Check for USB bus layer device (used for hub recursion)
			if (!device.GetDriverKeyName().empty() &&
				connectionInfo._driverKeyName == device.GetDriverKeyName())
			{
				spdlog::info("    (USB Bus layer device)");
				usbBusLayerDevice = device;
			}
		}

		// Select best GUID (prefer non-USB-Bus-Device GUIDs)
		if (!matchedGuids.empty())
		{
			GUID bestGuid = matchedGuids.front();
			if (matchedGuids.size() > 1)
			{
				for (const auto& guid : matchedGuids)
				{
					if (!IsEqualGUID(guid, GUID_DEVINTERFACE_USB_DEVICE)) {
						bestGuid = guid;
						break;
					}
				}
			}
			setupClassGuidMap.emplace(portNumber, bestGuid);
			spdlog::info("  Selected ClassGuid: {} (from {} candidate(s))",
				UtilConvert::WStringToUTF8(FormatGuid(bestGuid)), matchedGuids.size());
		}
		else
		{
			spdlog::warn("  No devices found matching pattern: {}", UtilConvert::WStringToUTF8(vidPidPattern));
		}

		// Handle hub recursion or config descriptor
		if (usbBusLayerDevice.has_value())
		{
			if (connectionInfo._deviceIsHub)
			{
				spdlog::info("  Recursively enumerating USB hub");
				DeviceInfo deviceInfo{ devInfoSet, usbBusLayerDevice->GetDevInfoData() };
				deviceInfo.PopulateUsbInfo();
				EnumeratePortsFromRootHub(deviceInfo.GetDevicePath(), allDevices, devInfoSet);
			}
			else
			{
				spdlog::debug("  Filling config descriptor for non-hub device");
				auto& mutableConnectionInfo = const_cast<HubConnectionInfo&>(connectionInfo);
				usbHub.FillConfigDescriptor(
					&mutableConnectionInfo._deviceDescriptor,
					connectionInfo._connectionIndex, 0);
			}
		}
	}

	// Build DeviceResultantInfo from USB device descriptions
	spdlog::info("EnumeratePortsFromRootHub: Processing {} device description(s)",
		usbHub.GetUsbDeviceDescriptionInfo().size());

	for (const auto& [portNum, deviceDescInfo] : usbHub.GetUsbDeviceDescriptionInfo())
	{
		spdlog::info("  Creating DeviceResultantInfo:");
		spdlog::info("    Manufacturer: {}", UtilConvert::WStringToUTF8(deviceDescInfo->GetManufacturer()));
		spdlog::info("    Product: {}", UtilConvert::WStringToUTF8(deviceDescInfo->GetProduct()));
		spdlog::info("    SerialNumber: {}", UtilConvert::WStringToUTF8(deviceDescInfo->GetSerialNumber()));

		DeviceResultantInfo resultInfo;
		resultInfo.SetManufacturer(deviceDescInfo->GetManufacturer());
		resultInfo.SetProduct(deviceDescInfo->GetProduct());

		// Fallback: Use registry DeviceDesc if USB string descriptors are empty
		if (resultInfo.GetManufacturer().empty() && resultInfo.GetProduct().empty())
		{
			auto vendorIt = vendorIdMap.find(portNum);
			auto productIt = productIdMap.find(portNum);

			if (vendorIt != vendorIdMap.end() && productIt != productIdMap.end())
			{
				std::wstring pattern = BuildVidPidPattern(vendorIt->second, productIt->second);

				for (const auto& device : allDevices)
				{
					if (ContainsIgnoreCase(device.GetHardwareId(), pattern))
					{
						std::wstring deviceDesc = device.GetDeviceDescription();
						if (!deviceDesc.empty())
						{
							resultInfo.SetProduct(deviceDesc);
							spdlog::info("    Registry fallback: Using DeviceDesc '{}'",
								UtilConvert::WStringToUTF8(deviceDesc));
							break;
						}
					}
				}
			}
		}

		// Try to find functional device GUID by product name match
		std::wstring productName = deviceDescInfo->GetProduct();
		if (productName.empty() && !resultInfo.GetProduct().empty()) {
			productName = resultInfo.GetProduct();
		}

		if (!productName.empty())
		{
			spdlog::debug("  Searching for functional device: {}", UtilConvert::WStringToUTF8(productName));
			for (const auto& device : allDevices)
			{
				std::wstring deviceDesc = device.GetDeviceDescription();
				if (!deviceDesc.empty() && deviceDesc.find(productName) != std::wstring::npos)
				{
					GUID functionalGuid = device.GetClassGuid();
					spdlog::info("  Found functional GUID: {} (Desc: {})",
						UtilConvert::WStringToUTF8(FormatGuid(functionalGuid)),
						UtilConvert::WStringToUTF8(deviceDesc));
					setupClassGuidMap.insert_or_assign(portNum, functionalGuid);
					break;
				}
			}
		}

		resultInfo.SetSerialNumber(deviceDescInfo->GetSerialNumber());

		// Set interface class
		if (UCHAR interfaceClass = deviceDescInfo->GetInterfaceClass(); interfaceClass != 0xFF)
		{
			resultInfo.SetInterfaceClass(interfaceClass);
			spdlog::info("    InterfaceClass: 0x{:02X} ({})", interfaceClass,
				UtilConvert::WStringToUTF8(UtilConvert::GetUsbClassNameByDescId(interfaceClass)));
		}

		// Set device class
		if (auto it = deviceClassMap.find(portNum); it != deviceClassMap.end())
		{
			resultInfo.SetDeviceClass(it->second);
			spdlog::info("    DeviceClass: 0x{:02X} ({})", it->second,
				UtilConvert::WStringToUTF8(UtilConvert::GetUsbClassNameByDescId(it->second)));
		}
		else
		{
			resultInfo.SetDeviceClass(0);
			spdlog::warn("    DeviceClass: Not found for port {}", portNum);
		}

		// Set vendor ID and name
		if (auto it = vendorIdMap.find(portNum); it != vendorIdMap.end())
		{
			resultInfo.SetVendorId(it->second);
			resultInfo.SetVendorName(GetVendorStringById(static_cast<USHORT>(it->second)));
			spdlog::info("    VendorId: 0x{:04X}", it->second);
			spdlog::info("    VendorName: {}", UtilConvert::WStringToUTF8(resultInfo.GetVendorName()));
		}

		// Set product ID
		if (auto it = productIdMap.find(portNum); it != productIdMap.end())
		{
			resultInfo.SetProductId(it->second);
			spdlog::info("    ProductId: 0x{:04X}", it->second);
		}

		// Set interface class name
		if (resultInfo.GetInterfaceClass() != 0xFF)
		{
			resultInfo.SetInterfaceClassName(UtilConvert::GetUsbClassNameByDescId(resultInfo.GetInterfaceClass()));
		}
		else if (resultInfo.GetDeviceClass() != 0)
		{
			resultInfo.SetInterfaceClassName(UtilConvert::GetUsbClassNameByDescId(resultInfo.GetDeviceClass()));
		}

		// Set Setup Class GUID
		if (auto it = setupClassGuidMap.find(portNum); it != setupClassGuidMap.end())
		{
			resultInfo.SetSetupClassGuid(it->second);
			spdlog::info("    SetupClassGuid: {}", UtilConvert::WStringToUTF8(FormatGuid(it->second)));
		}

		resultInfo.SetIsConnected(true);
		resultInfo.SetIsUsbDevice(true);

		AddDeviceInfo(std::move(resultInfo));
		spdlog::debug("  DeviceResultantInfo added");
	}
}

void DevicesManager::Impl::EnumerateUsbDevices()
{
	_devicesList.clear();

	spdlog::info("========================================");
	spdlog::info("EnumerateUsbDevices: Starting USB device enumeration");
	spdlog::info("========================================");

	DeviceEnumerator allDevicesEnumerator(
		GUID_DEVINTERFACE_USB_DEVICE,
		DIGCF_ALLCLASSES | DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	auto allUsbDevices = allDevicesEnumerator.GetDeviceInstances();
	spdlog::info("EnumerateUsbDevices: Found {} USB devices", allUsbDevices.size());

	DeviceEnumerator controllerEnumerator(
		GUID_CLASS_USB_HOST_CONTROLLER,
		DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	auto controllers = controllerEnumerator.GetDeviceInstances();
	spdlog::info("EnumerateUsbDevices: Found {} USB host controller(s)", controllers.size());

	for (auto& controller : controllers)
	{
		spdlog::info("Processing USB host controller");

		DeviceInfo deviceInfo{ controllerEnumerator.GetDevInfoSet(), controller.GetDevInfoData() };
		deviceInfo.PopulateUsbControllerInfo();

		std::wstring devicePath = deviceInfo.GetDevicePath();
		DeviceCommunication deviceCommunication(devicePath);
		UsbHostController hostController(devicePath, deviceCommunication);

		hostController.PopulateInfo();

		std::wstring rootHubPath = L"\\\\.\\" + hostController.GetRootHubName();
		spdlog::info("Root hub device: {}", UtilConvert::WStringToUTF8(rootHubPath));

		EnumeratePortsFromRootHub(rootHubPath, allUsbDevices, allDevicesEnumerator.GetDevInfoSet());
	}

	spdlog::info("========================================");
	spdlog::info("EnumerateUsbDevices: Complete - total devices: {}", _devicesList.size());
	spdlog::info("========================================");
}

void DevicesManager::Impl::EnumerateByDeviceClass(const GUID& deviceClassGuid)
{
	_devicesList.clear();

	spdlog::info("========================================");
	spdlog::info("EnumerateByDeviceClass: Starting enumeration");
	spdlog::info("========================================");

	try
	{
		DeviceEnumerator enumerator(deviceClassGuid, DIGCF_PRESENT);
		auto devices = enumerator.GetDeviceInstances();
		spdlog::info("EnumerateByDeviceClass: Found {} device(s)", devices.size());

		for (auto& devInfoData : devices)
		{
			try
			{
				DeviceProperty propReader(enumerator.GetDevInfoSet(), devInfoData.GetDevInfoData());
				DeviceResultantInfo resultInfo;

				// Retrieve device properties
				if (std::wstring desc; propReader.GetStringProperty(SPDRP_DEVICEDESC, desc))
				{
					resultInfo.SetDescription(desc);
					spdlog::debug("  Description: {}", UtilConvert::WStringToUTF8(desc));
				}

				if (std::wstring friendlyName; propReader.GetStringProperty(SPDRP_FRIENDLYNAME, friendlyName))
				{
					resultInfo.SetFriendlyName(friendlyName);
					spdlog::debug("  FriendlyName: {}", UtilConvert::WStringToUTF8(friendlyName));
				}

				if (std::wstring mfg; propReader.GetStringProperty(SPDRP_MFG, mfg))
				{
					resultInfo.SetManufacturer(mfg);
					spdlog::debug("  Manufacturer: {}", UtilConvert::WStringToUTF8(mfg));
				}

				if (std::wstring hwId; propReader.GetStringProperty(SPDRP_HARDWAREID, hwId))
				{
					resultInfo.SetDeviceId(hwId);
					spdlog::debug("  HardwareID: {}", UtilConvert::WStringToUTF8(hwId));
				}

				// Try to get USB device path
				DeviceInfo deviceInfo{ enumerator.GetDevInfoSet(), devInfoData.GetDevInfoData() };
				try
				{
					deviceInfo.PopulateUsbInfo();
					resultInfo.SetDevicePath(deviceInfo.GetDevicePath());
					resultInfo.SetIsUsbDevice(true);
					spdlog::debug("  DevicePath: {}", UtilConvert::WStringToUTF8(deviceInfo.GetDevicePath()));
				}
				catch (const std::exception&)
				{
					resultInfo.SetIsUsbDevice(false);
					spdlog::debug("  Not a USB device");
				}

				resultInfo.SetSetupClassGuid(deviceClassGuid);
				resultInfo.SetIsConnected(true);

				AddDeviceInfo(std::move(resultInfo));
				spdlog::debug("  Device added");
			}
			catch (const std::exception& e)
			{
				spdlog::warn("  Failed to process device: {}", e.what());
			}
		}

		spdlog::info("========================================");
		spdlog::info("EnumerateByDeviceClass: Complete - total: {}", _devicesList.size());
		spdlog::info("========================================");
	}
	catch (const std::exception& e)
	{
		spdlog::error("EnumerateByDeviceClass: Exception: {}", e.what());
		throw;
	}
}

// DevicesManager Public Interface
DevicesManager::DevicesManager()
	: pImpl{ std::make_unique<Impl>() }
{
}

DevicesManager::~DevicesManager() = default;
DevicesManager::DevicesManager(DevicesManager&&) noexcept = default;
DevicesManager& DevicesManager::operator=(DevicesManager&&) noexcept = default;

void DevicesManager::EnumerateUsbDevices()
{
	pImpl->EnumerateUsbDevices();
}

void DevicesManager::EnumerateByDeviceClass(const GUID& deviceClassGuid)
{
	pImpl->EnumerateByDeviceClass(deviceClassGuid);
}

void DevicesManager::AddDeviceInfo(DeviceResultantInfo deviceResultantInfo)
{
	pImpl->AddDeviceInfo(std::move(deviceResultantInfo));
}

const std::vector<DeviceResultantInfo>& DevicesManager::GetDevices() const noexcept
{
	return pImpl->GetDevices();
}

size_t DevicesManager::GetDeviceCount() const noexcept
{
	return pImpl->GetDeviceCount();
}

void DevicesManager::ClearDevices() noexcept
{
	pImpl->ClearDevices();
}

}
