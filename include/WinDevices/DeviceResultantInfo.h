#pragma once

#include <Windows.h>
#include <string>

/// @brief Encapsulates device information retrieved from USB or device class enumeration.
///
/// This class serves as the primary data transfer object for device information
/// collected during enumeration. It contains both USB-specific information
/// (VID/PID, device class, descriptors) and Windows-specific information
/// (device path, setup class GUID, friendly name).
///
/// **USB Enumeration Fields** (populated by EnumerateUsbDevices):
/// - manufacturer_: From USB string descriptor (iManufacturer)
/// - product_: From USB string descriptor (iProduct)
/// - serialNumber_: From USB string descriptor (iSerialNumber)
/// - vendorId_/productId_: From USB device descriptor
/// - deviceClass_: From USB device descriptor (bDeviceClass)
/// - interfaceClass_: From USB interface descriptor (bInterfaceClass)
/// - vendorName_: Looked up from USB-IF vendor database
/// - interfaceClassName_: Human-readable name for the interface class
///
/// **Device Class Enumeration Fields** (populated by EnumerateByDeviceClass):
/// - description_: From SPDRP_DEVICEDESC registry property
/// - friendlyName_: From SPDRP_FRIENDLYNAME registry property
/// - manufacturer_: From SPDRP_MFG registry property
/// - deviceId_: Hardware ID from SPDRP_HARDWAREID
/// - devicePath_: Windows device path for DeviceIoControl access
/// - setupClassGuid_: Windows Device Setup Class GUID
///
/// @note This class uses value semantics and is fully copyable/movable.
class DeviceResultantInfo
{
public:
	DeviceResultantInfo() = default;

	// Value semantics - fully copyable and movable
	DeviceResultantInfo(const DeviceResultantInfo&) = default;
	DeviceResultantInfo& operator=(const DeviceResultantInfo&) = default;
	DeviceResultantInfo(DeviceResultantInfo&&) noexcept = default;
	DeviceResultantInfo& operator=(DeviceResultantInfo&&) noexcept = default;
	~DeviceResultantInfo() = default;

	// ==================== String Getters ====================

	/// @brief Returns the manufacturer name (from USB descriptor or registry).
	[[nodiscard]] const std::wstring& GetManufacturer() const noexcept { return manufacturer_; }

	/// @brief Returns the product name (from USB descriptor or registry fallback).
	[[nodiscard]] const std::wstring& GetProduct() const noexcept { return product_; }

	/// @brief Returns the serial number (from USB string descriptor).
	[[nodiscard]] const std::wstring& GetSerialNumber() const noexcept { return serialNumber_; }

	/// @brief Returns the device description (from Windows registry SPDRP_DEVICEDESC).
	[[nodiscard]] const std::wstring& GetDescription() const noexcept { return description_; }

	/// @brief Returns the hardware ID (e.g., "USB\\VID_0951&PID_172B").
	[[nodiscard]] const std::wstring& GetDeviceId() const noexcept { return deviceId_; }

	/// @brief Returns the friendly name (from Windows registry SPDRP_FRIENDLYNAME).
	[[nodiscard]] const std::wstring& GetFriendlyName() const noexcept { return friendlyName_; }

	/// @brief Returns the Windows device path for DeviceIoControl access.
	[[nodiscard]] const std::wstring& GetDevicePath() const noexcept { return devicePath_; }

	/// @brief Returns the USB-IF registered vendor name (looked up by VID).
	[[nodiscard]] const std::wstring& GetVendorName() const noexcept { return vendorName_; }

	/// @brief Returns the human-readable USB interface class name.
	[[nodiscard]] const std::wstring& GetInterfaceClassName() const noexcept { return interfaceClassName_; }

	// ==================== Numeric/Boolean Getters ====================

	/// @brief Returns the USB device class (bDeviceClass from device descriptor).
	/// @return Device class code (e.g., 0x00=Interface-defined, 0x08=Mass Storage, 0x09=Hub).
	[[nodiscard]] UCHAR GetDeviceClass() const noexcept { return deviceClass_; }

	/// @brief Returns the USB interface class (bInterfaceClass from interface descriptor).
	/// @return Interface class code, or 0xFF if not set.
	[[nodiscard]] UCHAR GetInterfaceClass() const noexcept { return interfaceClass_; }

	/// @brief Returns the Windows Device Setup Class GUID.
	[[nodiscard]] const GUID& GetSetupClassGuid() const noexcept { return setupClassGuid_; }

	/// @brief Returns the USB Vendor ID (VID).
	[[nodiscard]] unsigned int GetVendorId() const noexcept { return vendorId_; }

	/// @brief Returns the USB Product ID (PID).
	[[nodiscard]] unsigned int GetProductId() const noexcept { return productId_; }

	/// @brief Returns true if this device was identified as a USB device.
	[[nodiscard]] bool IsUsbDevice() const noexcept { return isUsbDevice_; }

	/// @brief Returns true if the device is currently connected.
	[[nodiscard]] bool IsConnected() const noexcept { return isConnected_; }

	// ==================== Setters ====================

	void SetManufacturer(std::wstring value) { manufacturer_ = std::move(value); }
	void SetProduct(std::wstring value) { product_ = std::move(value); }
	void SetSerialNumber(std::wstring value) { serialNumber_ = std::move(value); }
	void SetDescription(std::wstring value) { description_ = std::move(value); }
	void SetDeviceId(std::wstring value) { deviceId_ = std::move(value); }
	void SetFriendlyName(std::wstring value) { friendlyName_ = std::move(value); }
	void SetDevicePath(std::wstring value) { devicePath_ = std::move(value); }
	void SetVendorName(std::wstring value) { vendorName_ = std::move(value); }
	void SetInterfaceClassName(std::wstring value) { interfaceClassName_ = std::move(value); }

	void SetDeviceClass(UCHAR value) noexcept { deviceClass_ = value; }
	void SetInterfaceClass(UCHAR value) noexcept { interfaceClass_ = value; }
	void SetSetupClassGuid(const GUID& value) noexcept { setupClassGuid_ = value; }
	void SetVendorId(unsigned int value) noexcept { vendorId_ = value; }
	void SetProductId(unsigned int value) noexcept { productId_ = value; }
	void SetIsUsbDevice(bool value) noexcept { isUsbDevice_ = value; }
	void SetIsConnected(bool value) noexcept { isConnected_ = value; }

private:
	std::wstring manufacturer_;
	std::wstring product_;
	std::wstring serialNumber_;
	std::wstring description_;
	std::wstring deviceId_;
	std::wstring friendlyName_;
	std::wstring devicePath_;
	std::wstring vendorName_;
	std::wstring interfaceClassName_;

	UCHAR deviceClass_ = 0;
	UCHAR interfaceClass_ = 0xFF;  // 0xFF = not set
	GUID setupClassGuid_ = { 0 };
	unsigned int vendorId_ = 0;
	unsigned int productId_ = 0;
	bool isUsbDevice_ = false;
	bool isConnected_ = false;
};
