#pragma once

#include <string>
#include <string_view>

namespace KDM
{
	/// <summary>
	/// Utility class for common conversions (USB class names, hex formatting, string encoding).
	/// All methods are static - this is a stateless utility class.
	/// </summary>
	class UtilConvert
	{
	public:
		// Delete constructor - this is a static utility class
		UtilConvert() = delete;

		/// <summary>
		/// Gets the human-readable name for a USB device class code.
		/// </summary>
		/// <param name="DeviceClass">USB device class code (0x00-0xFF)</param>
		/// <returns>Human-readable class name (e.g., "Mass Storage", "HID")</returns>
		[[nodiscard]] static std::wstring GetUsbClassNameByDescId(UCHAR DeviceClass);

		/// <summary>
		/// Formats an integer value as a hex string with specified width.
		/// </summary>
		/// <param name="Value">Value to convert</param>
		/// <param name="BytesNumber">Width of hex output (e.g., 4 for "0x1234")</param>
		/// <returns>Hex string (e.g., "0x0403" for Value=1027, BytesNumber=4)</returns>
		[[nodiscard]] static std::wstring GetHexIdAsString(USHORT Value, USHORT BytesNumber);

		/// <summary>
		/// Gets the base class name for a USB class code.
		/// </summary>
		[[nodiscard]] static std::wstring GetBaseClassById(USHORT value);

		/// <summary>
		/// Converts a wide string to UTF-8 encoded string.
		/// </summary>
		/// <param name="wstr">Wide string to convert</param>
		/// <returns>UTF-8 encoded string</returns>
		[[nodiscard]] static std::string WStringToUTF8(std::wstring_view wstr);

		/// <summary>
		/// Converts a UTF-8 encoded string to wide string.
		/// </summary>
		/// <param name="str">UTF-8 encoded string to convert</param>
		/// <returns>Wide string</returns>
		[[nodiscard]] static std::wstring UTF8ToWString(std::string_view str);
	};
}