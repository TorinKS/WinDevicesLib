
#pragma once
#include "HubNodeInfo.h"
#include "HubNodeInfoEx.h"
#include "HubNodeCapabilitiesEx.h"
#include "HubPortInfo.h"
#include "HubConnectionInfo.h"
#include "IDeviceCommunication.h"
#include "UsbDeviceDescriptorInfo.h"
#include <memory>

namespace KDM
{
	class DeviceCommunication;
	class UsbPortInfo;

	/// <summary>
	/// Represents a USB Hub device and manages its port enumeration.
	/// Uses RAII and smart pointers for automatic resource management.
	/// Supports dependency injection for IDeviceCommunication to enable unit testing.
	/// </summary>
	class UsbHub
	{
	public:
		/// <summary>
		/// Constructs a UsbHub with a new DeviceCommunication instance (default behavior).
		/// </summary>
		/// <param name="hubName">The device path of the USB hub</param>
		explicit UsbHub(std::wstring hubName);

		/// <summary>
		/// Constructs a UsbHub with an injected IDeviceCommunication instance (for testing).
		/// </summary>
		/// <param name="hubName">The device path of the USB hub</param>
		/// <param name="deviceCommunication">An injected IDeviceCommunication implementation</param>
		UsbHub(std::wstring hubName, std::unique_ptr<IDeviceCommunication> deviceCommunication);

		~UsbHub();

		// Rule of Five: Delete copy operations (non-copyable due to unique_ptr members)
		UsbHub(const UsbHub&) = delete;
		UsbHub& operator=(const UsbHub&) = delete;

		// Allow move operations
		UsbHub(UsbHub&&) noexcept = default;
		UsbHub& operator=(UsbHub&&) noexcept = default;

		void SetNumberOfPorts(ULONG NumberOfPorts);
		void SetDeviceCommunication(DeviceCommunication& DeviceCommunication);
		void PopulateInfo();
		
		[[nodiscard]] IDeviceCommunication* GetDeviceCommunication() const noexcept;

		void FillConfigDescriptor(USB_DEVICE_DESCRIPTOR* UsbDeviceDescriptor,
			ULONG   ConnectionIndex, UCHAR   DescriptorIndex);
		bool AreUsbDescriptorsCorrect(USB_DEVICE_DESCRIPTOR* UsbDeviceDescriptor,
			PUSB_CONFIGURATION_DESCRIPTOR UsbConfigurationDescriptor);

		bool GetAllStringDescriptors(
			ULONG                           ConnectionIndex,
			PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
			PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
			UsbDeviceDescriptorInfo* DeviceInfo
		);

		[[nodiscard]] const std::map<size_t, HubPortInfo>& GetHubPortInfo() const noexcept;
		[[nodiscard]] const std::map<size_t, HubConnectionInfo>& GetPortConnectionInfo() const noexcept;
		[[nodiscard]] const std::map<size_t, std::unique_ptr<UsbDeviceDescriptorInfo>>& GetUsbDeviceDescriptionInfo() const noexcept;


		// const PUSB_NODE_INFORMATION& GetHubInfo() const;
		// const PUSB_HUB_INFORMATION_EX& GetHubInfoEx() const;

	private:
		// Custom deleter for USB_DESCRIPTOR_REQUEST allocated with new BYTE[]
		struct UsbDescriptorRequestDeleter {
			void operator()(PUSB_DESCRIPTOR_REQUEST ptr) const {
				delete[] reinterpret_cast<BYTE*>(ptr);
			}
		};

		using UsbDescriptorRequestPtr = std::unique_ptr<USB_DESCRIPTOR_REQUEST, UsbDescriptorRequestDeleter>;

		std::wstring _hubName;
		std::vector<UsbPortInfo> _usbPortInfoList;
		ULONG _numberOfPorts = 0;

		// Smart pointer for IDeviceCommunication - automatic cleanup
		// Supports dependency injection for unit testing
		std::unique_ptr<IDeviceCommunication> _pDeviceCommunication;

		std::map<size_t, HubPortInfo> _hubPortConnectorProperties;
		std::map<size_t, HubConnectionInfo> _hubPortConnectionInfo;

		// Smart pointer maps - automatic cleanup, no manual delete needed
		std::map<size_t, UsbDescriptorRequestPtr> _portUsbConfigurationDescriptor;
		std::map<size_t, std::unique_ptr<UsbDeviceDescriptorInfo>> _usbDeviceDescriptionInfo;
	};
}