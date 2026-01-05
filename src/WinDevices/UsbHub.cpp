#include "pch.h"
#include "UsbPortInfo.h"
#include "UsbHub.h"
#include "DeviceCommunication.h"
#include <spdlog/spdlog.h>


namespace KDM
{
	/// <summary>
	/// Constructs a USB Hub object and initializes communication.
	/// Uses smart pointer for automatic resource management.
	/// </summary>
	/// <param name="hubName">The device path of the USB hub</param>
	UsbHub::UsbHub(std::wstring hubName) :
		_hubName(std::move(hubName)),
		_numberOfPorts(0)
	{
		_pDeviceCommunication = std::make_unique<DeviceCommunication>(_hubName);
	}

	/// <summary>
	/// Constructs a USB Hub object with an injected IDeviceCommunication.
	/// Used for dependency injection in unit tests.
	/// </summary>
	/// <param name="hubName">The device path of the USB hub</param>
	/// <param name="deviceCommunication">An injected IDeviceCommunication implementation</param>
	UsbHub::UsbHub(std::wstring hubName, std::unique_ptr<IDeviceCommunication> deviceCommunication) :
		_hubName(std::move(hubName)),
		_numberOfPorts(0),
		_pDeviceCommunication(std::move(deviceCommunication))
	{
	}

	/// <summary>
	/// Destructor - smart pointers handle all cleanup automatically (RAII).
	/// No manual delete calls needed.
	/// </summary>
	UsbHub::~UsbHub() = default;


	void UsbHub::PopulateInfo()
	{
		HubNodeInfo nodeInfo;
		HubNodeInfoEx nodeInfoEx;
		HubNodeCapabilitiesEx hubCapabilityEx;

		_pDeviceCommunication->GetUsbHubNodeInformation(nodeInfo);
		_pDeviceCommunication->GetUsbHubNodeInformationEx(nodeInfoEx);
		_pDeviceCommunication->GetUsbHubNodeCapabilitiesEx(hubCapabilityEx);

		_numberOfPorts = nodeInfo.numbersOfPorts;

		// getting port Connector properties
		_pDeviceCommunication->EnumeratePorts(_numberOfPorts, _hubPortConnectorProperties);
		//_pDeviceCommunication->EnumeratePortsConnectionInfo(_numberOfPorts );

		_pDeviceCommunication->EnumeratePortsConnectionInfo(_numberOfPorts, _hubPortConnectionInfo);

	}


	void UsbHub::SetNumberOfPorts(ULONG NumberOfPorts)
	{
		_numberOfPorts = NumberOfPorts;
	}

	void UsbHub::SetDeviceCommunication(DeviceCommunication& DeviceCommunication)
	{

	}

	const std::map<size_t, HubPortInfo>& UsbHub::GetHubPortInfo() const noexcept
	{
		return _hubPortConnectorProperties;
	}

	const std::map<size_t, HubConnectionInfo>& UsbHub::GetPortConnectionInfo() const noexcept
	{
		return _hubPortConnectionInfo;
	}

	const std::map<size_t, std::unique_ptr<UsbDeviceDescriptorInfo>>& UsbHub::GetUsbDeviceDescriptionInfo() const noexcept
	{
		return _usbDeviceDescriptionInfo;
	}


	IDeviceCommunication* UsbHub::GetDeviceCommunication() const noexcept
	{
		return _pDeviceCommunication.get();
	}

	/// <summary>
	/// Fills configuration descriptor for a USB device.
	/// Uses smart pointers for automatic memory management.
	/// </summary>
	/// <param name="UsbDeviceDescriptor">Pointer to USB device descriptor</param>
	/// <param name="ConnectionIndex">Connection index on the hub</param>
	/// <param name="DescriptorIndex">Descriptor index</param>

	void UsbHub::FillConfigDescriptor(USB_DEVICE_DESCRIPTOR* UsbDeviceDescriptor,
		ULONG   ConnectionIndex, UCHAR   DescriptorIndex)
	{
		PUSB_DESCRIPTOR_REQUEST pUsbDescriptorRequest =
			_pDeviceCommunication->GetConfigDescriptor(ConnectionIndex, 0);

		if (pUsbDescriptorRequest != nullptr)
		{
			// Use smart pointer for automatic cleanup
			auto pUsbDeviceDescriptorInfo = std::make_unique<UsbDeviceDescriptorInfo>();

			// Extract interface class from the first interface descriptor
			// This should be done regardless of whether string descriptors are available
			PUSB_CONFIGURATION_DESCRIPTOR configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(pUsbDescriptorRequest + 1);
			PUSB_COMMON_DESCRIPTOR commonDesc = (PUSB_COMMON_DESCRIPTOR)configDesc;
			PUCHAR descEnd = (PUCHAR)configDesc + configDesc->wTotalLength;

			// Skip the configuration descriptor itself
			commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);

			// Find the first interface descriptor
			while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
				   (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
			{
				if (commonDesc->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE)
				{
					PUSB_INTERFACE_DESCRIPTOR interfaceDesc = (PUSB_INTERFACE_DESCRIPTOR)commonDesc;
					pUsbDeviceDescriptorInfo->SetInterfaceClass(interfaceDesc->bInterfaceClass);
					break;
				}
				commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
			}

			// Extract string descriptors if available
			if (AreUsbDescriptorsCorrect(UsbDeviceDescriptor,
				(PUSB_CONFIGURATION_DESCRIPTOR)(pUsbDescriptorRequest + 1))) // points it to USB_CONFIGURATION_DESCRIPTOR
			{
				GetAllStringDescriptors(
					ConnectionIndex,
					UsbDeviceDescriptor,
					(PUSB_CONFIGURATION_DESCRIPTOR)(pUsbDescriptorRequest + 1),
					pUsbDeviceDescriptorInfo.get()
				);
			}
			// else: No string descriptors available for this device (normal for some devices)

			// Wrap raw pointer in smart pointer with custom deleter
			UsbDescriptorRequestPtr descriptorPtr(pUsbDescriptorRequest);
			_portUsbConfigurationDescriptor.emplace(ConnectionIndex, std::move(descriptorPtr));

			// Move ownership to the map
			_usbDeviceDescriptionInfo.emplace(ConnectionIndex, std::move(pUsbDeviceDescriptorInfo));
		}
	}

	bool UsbHub::AreUsbDescriptorsCorrect(USB_DEVICE_DESCRIPTOR* UsbDeviceDescriptor,
		PUSB_CONFIGURATION_DESCRIPTOR UsbConfigurationDescriptor
	)
	{
		PUCHAR                  descEnd = nullptr;
		PUSB_COMMON_DESCRIPTOR  commonDesc = nullptr;


		// TODO commented temporary
		
		if (UsbDeviceDescriptor->iManufacturer || UsbDeviceDescriptor->iProduct ||
			UsbDeviceDescriptor->iSerialNumber
			)
		{
			return true;
		}
		


		// set to the first byte after end of USB_CONFIGURATION_DESCRIPTOR structure
		descEnd = (PUCHAR)UsbConfigurationDescriptor + UsbConfigurationDescriptor->wTotalLength;

		commonDesc = (PUSB_COMMON_DESCRIPTOR)UsbConfigurationDescriptor;

		/*
			USB_COMMON_DESCRIPTOR and USB_CONFIGURATION_DESCRIPTOR have the same 2 first elements
				UCHAR   bLength;
				UCHAR   bDescriptorType;
		*/


		while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
			(PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
		{
			// here we interpreter first 2 UCHAR as bLength and bDescriptorType of USB_COMMON_DESCRIPTOR
			// and check that their values are correct from the USB_COMMON_DESCRIPTOR point of view
			switch (commonDesc->bDescriptorType)
			{
			case USB_CONFIGURATION_DESCRIPTOR_TYPE:
			case USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR_TYPE:
				if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
				{
					return false;

				}

				if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
				{
					return true;

				}
				commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
				continue; // try to check next element
			case USB_INTERFACE_DESCRIPTOR_TYPE:
				if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
					commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
				{

					// original USBView terminate app if value of USB_INTERFACE_DESCRIPTOR_TYPE
					// is not valid, we just return false, allowing code to get info about
					// another usb devices...
					// TODO: add trace logging here
					return false;
				}

				if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
				{
					return true;
				}
				commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
				continue;

			default:
				commonDesc = (PUSB_COMMON_DESCRIPTOR)((PUCHAR)commonDesc + commonDesc->bLength);
				continue;
			}

			break;
			// this break is present in original usbview app. This is a bug or a hack as it checks only 
			// first USB_COMMON_DESCRIPTOR el. 
			// also typically, first check of iManufacturer or iProduct or iSerial returns true

		}

		return false;
	}

	// Custom deleter for STRING_DESCRIPTOR_NODE allocated with new BYTE[]
	struct StringDescriptorDeleter {
		void operator()(PSTRING_DESCRIPTOR_NODE ptr) const {
			delete[] reinterpret_cast<BYTE*>(ptr);
		}
	};
	using StringDescriptorPtr = std::unique_ptr<STRING_DESCRIPTOR_NODE, StringDescriptorDeleter>;

	bool UsbHub::GetAllStringDescriptors(ULONG ConnectionIndex,
		PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
		PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
		UsbDeviceDescriptorInfo* DeviceInfo
	)
	{
		ULONG                   numLanguageIDs = 0;
		USHORT* languageIDs = nullptr;

		// first we get supported language ids, see for an example USB 3.0 specification
		// 9.6.8

		StringDescriptorPtr supportedLanguagesString(
			_pDeviceCommunication->GetStringDescriptor(ConnectionIndex, 0, 0));

		USHORT defaultLanguageID = 0x0409; // EN-US

		// If the device doesn't provide language descriptors, use default EN-US (0x0409)
		// Microsoft states that Windows always uses 0x0409 for serial numbers:
		// https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-faq--introductory-level
		// "A USB device indicates the presence of a serial number by setting the iSerialNumber field
		// of the USB device descriptor to the serial number's string index. To retrieve the
		// serial number, Windows issues a string request with the language identifier (LANGID)
		// set to 0x0409 (U.S. English). Windows always uses this LANGID to retrieve USB serial numbers,
		// even for versions of Windows that are localized for other languages"

		if (!supportedLanguagesString)
		{
			spdlog::debug("No language descriptor available, using default EN-US (0x0409)");
			// Use default language ID and continue processing
			languageIDs = &defaultLanguageID;
			numLanguageIDs = 1;
		}
		else
		{
			numLanguageIDs = (supportedLanguagesString->StringDescriptor->bLength - 2) / 2;
			languageIDs = reinterpret_cast<USHORT*>(&supportedLanguagesString->StringDescriptor->bString[0]);
		}

		std::wstring iManufacturer;
		std::wstring iProduct;
		std::wstring iSerialNumber;
		std::wstring bDeviceClass;

		if (DeviceDesc->bDeviceClass)
		{
			StringDescriptorPtr pDescriptorNode(
				_pDeviceCommunication->GetStringDescriptor(ConnectionIndex,
					DeviceDesc->bDeviceClass, languageIDs[0]));

			if (pDescriptorNode)
			{
				std::wcout << L"\tbDeviceClass: " << (WCHAR*)(pDescriptorNode->StringDescriptor->bString) << std::endl;
				bDeviceClass.assign((WCHAR*)(pDescriptorNode->StringDescriptor->bString));
			}
		}

		if (DeviceDesc->iManufacturer)
		{
			// Get data only for en-US (code 1033, 0x409)
			StringDescriptorPtr pDescriptorNode(
				_pDeviceCommunication->GetStringDescriptor(ConnectionIndex,
					DeviceDesc->iManufacturer, languageIDs[0]));

			if (pDescriptorNode)
			{
				iManufacturer.assign((WCHAR*)(pDescriptorNode->StringDescriptor->bString));
				spdlog::debug("GetAllStringDescriptors: Successfully retrieved manufacturer string");
			}
		}

		if (DeviceDesc->iProduct)
		{
			// Get data only for en-US (code 1033, 0x409)
			StringDescriptorPtr pDescriptorNode(
				_pDeviceCommunication->GetStringDescriptor(ConnectionIndex,
					DeviceDesc->iProduct, languageIDs[0]));

			if (pDescriptorNode)
			{
				iProduct.assign((WCHAR*)pDescriptorNode->StringDescriptor->bString);
			}
		}

		if (DeviceDesc->iSerialNumber)
		{
			// Get data only for en-US (code 1033, 0x409)
			StringDescriptorPtr pDescriptorNode(
				_pDeviceCommunication->GetStringDescriptor(ConnectionIndex,
					DeviceDesc->iSerialNumber, languageIDs[0]));

			if (pDescriptorNode)
			{
				iSerialNumber.assign((WCHAR*)pDescriptorNode->StringDescriptor->bString);
			}
		}

		DeviceInfo->SetUsbDeviceInfo(iManufacturer, iProduct, iSerialNumber);

		return true;
	}

}