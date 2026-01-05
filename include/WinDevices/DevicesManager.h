#pragma once
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <Windows.h>
#include <memory>
#include <vector>

// Forward declaration - DeviceResultantInfo is in global namespace
class DeviceResultantInfo;

namespace KDM
{
	/// @brief High-level manager for enumerating and accessing USB devices.
	///
	/// DevicesManager provides a simple interface for discovering USB devices
	/// connected to the system. It handles the complexity of USB hub traversal,
	/// descriptor retrieval, and device property collection internally.
	///
	/// Two enumeration modes are supported:
	/// 1. **USB Enumeration** (EnumerateUsbDevices): Traverses the USB bus hierarchy
	///    starting from host controllers, through root hubs and external hubs,
	///    collecting detailed USB device information including VID/PID, device class,
	///    manufacturer, product name, and serial number.
	///
	/// 2. **Device Class Enumeration** (EnumerateByDeviceClass): Uses Windows SetupAPI
	///    to enumerate devices by their Device Setup Class GUID (e.g., keyboard, mouse,
	///    disk drive, network adapter).
	///
	/// @note This class uses the PIMPL idiom for ABI stability and to hide
	/// implementation details. It is move-only due to owning internal resources.
	///
	/// @example USB device enumeration:
	/// @code
	/// KDM::DevicesManager manager;
	/// manager.EnumerateUsbDevices();
	/// for (const auto& device : manager.GetDevices()) {
	///     std::wcout << L"Product: " << device.GetProduct()
	///                << L" VID: " << std::hex << device.GetVendorId()
	///                << L" PID: " << device.GetProductId() << std::endl;
	/// }
	/// @endcode
	///
	/// @example Device class enumeration (keyboards):
	/// @code
	/// KDM::DevicesManager manager;
	/// manager.EnumerateByDeviceClass(GUID_DEVCLASS_KEYBOARD);
	/// for (const auto& device : manager.GetDevices()) {
	///     std::wcout << L"Keyboard: " << device.GetDescription() << std::endl;
	/// }
	/// @endcode
	class DevicesManager
	{
	public:
		/// @brief Constructs a new DevicesManager instance.
		DevicesManager();

		/// @brief Destructor (defined in .cpp for PIMPL).
		~DevicesManager();

		// Move-only semantics (PIMPL with unique_ptr)
		DevicesManager(const DevicesManager&) = delete;
		DevicesManager& operator=(const DevicesManager&) = delete;
		DevicesManager(DevicesManager&&) noexcept;
		DevicesManager& operator=(DevicesManager&&) noexcept;

		/// @brief Enumerates all USB devices connected to the system.
		///
		/// This method performs a full USB bus traversal:
		/// 1. Enumerates all USB host controllers
		/// 2. For each controller, opens the root hub
		/// 3. Recursively traverses all hubs, collecting device information
		/// 4. Retrieves USB descriptors (device, configuration, string)
		/// 5. Correlates devices with Windows SetupAPI information
		///
		/// Results are stored internally and can be accessed via GetDevices().
		/// Calling this method clears any previously enumerated devices.
		///
		/// @note This operation may take some time on systems with many USB devices.
		void EnumerateUsbDevices();

		/// @brief Enumerates devices by Windows Device Setup Class GUID.
		///
		/// This method uses SetupAPI to enumerate devices belonging to a specific
		/// device class. Common GUIDs include:
		/// - GUID_DEVCLASS_KEYBOARD: Keyboard devices
		/// - GUID_DEVCLASS_MOUSE: Mouse/pointing devices
		/// - GUID_DEVCLASS_DISKDRIVE: Disk drives
		/// - GUID_DEVCLASS_NET: Network adapters
		/// - GUID_DEVCLASS_DISPLAY: Display adapters
		/// - GUID_DEVCLASS_USB: USB controllers
		/// - GUID_DEVCLASS_HIDCLASS: HID devices
		///
		/// @param deviceClassGuid The Device Setup Class GUID to filter by.
		/// @see https://docs.microsoft.com/en-us/windows-hardware/drivers/install/system-defined-device-setup-classes-available-to-vendors
		void EnumerateByDeviceClass(const GUID& deviceClassGuid);

		/// @brief Manually adds a device to the internal device list.
		/// @param deviceResultantInfo Device information to add (moved).
		void AddDeviceInfo(DeviceResultantInfo deviceResultantInfo);

		/// @brief Returns a const reference to the enumerated devices.
		/// @return Reference to the internal device vector (avoids copy).
		[[nodiscard]] const std::vector<DeviceResultantInfo>& GetDevices() const noexcept;

		/// @brief Returns the number of enumerated devices.
		/// @return Device count.
		[[nodiscard]] size_t GetDeviceCount() const noexcept;

		/// @brief Clears all enumerated devices from the internal list.
		void ClearDevices() noexcept;

	private:
		class Impl;
		std::unique_ptr<Impl> pImpl;
	};
}
