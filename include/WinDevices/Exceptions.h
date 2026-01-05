#pragma once

#include <stdexcept>
#include <string>
#include <Windows.h>

namespace KDM
{
    /// <summary>
    /// Base exception class for all device-related errors.
    /// </summary>
    class DeviceException : public std::runtime_error
    {
    public:
        explicit DeviceException(const std::string& message)
            : std::runtime_error(message) {}

        explicit DeviceException(const char* message)
            : std::runtime_error(message) {}
    };

    /// <summary>
    /// Exception thrown when device enumeration fails.
    /// </summary>
    class DeviceEnumerationException : public DeviceException
    {
    public:
        explicit DeviceEnumerationException(const std::string& message, DWORD errorCode = 0)
            : DeviceException(message), errorCode_(errorCode) {}

        [[nodiscard]] DWORD GetErrorCode() const noexcept { return errorCode_; }

    private:
        DWORD errorCode_ = 0;
    };

    /// <summary>
    /// Exception thrown when device I/O operations fail.
    /// </summary>
    class DeviceIoException : public DeviceException
    {
    public:
        explicit DeviceIoException(const std::string& message, DWORD errorCode = 0)
            : DeviceException(message), errorCode_(errorCode) {}

        [[nodiscard]] DWORD GetErrorCode() const noexcept { return errorCode_; }

    private:
        DWORD errorCode_ = 0;
    };

    /// <summary>
    /// Exception thrown when invalid arguments are passed to device methods.
    /// </summary>
    class InvalidDeviceArgumentException : public DeviceException
    {
    public:
        explicit InvalidDeviceArgumentException(const std::string& message)
            : DeviceException(message) {}

        explicit InvalidDeviceArgumentException(const char* message)
            : DeviceException(message) {}
    };

    /// <summary>
    /// Exception thrown when a device handle is invalid or not opened.
    /// </summary>
    class InvalidDeviceHandleException : public DeviceException
    {
    public:
        explicit InvalidDeviceHandleException(const std::string& message = "Invalid device handle")
            : DeviceException(message) {}
    };

}  // namespace KDM
