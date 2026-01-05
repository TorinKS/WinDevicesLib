#pragma once

#include <optional>

/// <summary>
/// Stores USB device descriptor string information retrieved from USB string descriptors.
/// Uses std::optional for interface class to properly represent "not set" state.
/// </summary>
class UsbDeviceDescriptorInfo
{
public:
	UsbDeviceDescriptorInfo() = default;

	// Rule of Five - use defaults for value-semantic class
	UsbDeviceDescriptorInfo(const UsbDeviceDescriptorInfo&) = default;
	UsbDeviceDescriptorInfo& operator=(const UsbDeviceDescriptorInfo&) = default;
	UsbDeviceDescriptorInfo(UsbDeviceDescriptorInfo&&) noexcept = default;
	UsbDeviceDescriptorInfo& operator=(UsbDeviceDescriptorInfo&&) noexcept = default;
	~UsbDeviceDescriptorInfo() = default;

	void SetUsbDeviceInfo(std::wstring manufacturer,
		std::wstring product,
		std::wstring serialNumber);

	void SetInterfaceClass(UCHAR interfaceClass) { _interfaceClass = interfaceClass; }

	// Legacy getters (return copy for backward compatibility)
	[[nodiscard]] std::wstring GetManufacturer() const { return _manufacturer; }
	[[nodiscard]] std::wstring GetProduct() const { return _product; }
	[[nodiscard]] std::wstring GetSerialNumber() const { return _serialNumber; }

	/// <summary>
	/// Gets the USB interface class code.
	/// </summary>
	/// <returns>Interface class if set, 0xFF if not set (legacy behavior)</returns>
	[[nodiscard]] UCHAR GetInterfaceClass() const { return _interfaceClass.value_or(0xFF); }

	/// <summary>
	/// Gets the USB interface class code as optional.
	/// </summary>
	/// <returns>Interface class if set, std::nullopt if not retrieved</returns>
	[[nodiscard]] std::optional<UCHAR> GetInterfaceClassOptional() const noexcept { return _interfaceClass; }

	/// <summary>
	/// Checks if the interface class has been set.
	/// </summary>
	/// <returns>true if interface class was retrieved from USB descriptor</returns>
	[[nodiscard]] bool HasInterfaceClass() const noexcept { return _interfaceClass.has_value(); }

private:
	std::wstring _manufacturer;
	std::wstring _product;
	std::wstring _serialNumber;
	std::optional<UCHAR> _interfaceClass = std::nullopt;
};
