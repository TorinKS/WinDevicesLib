using FluentAssertions;
using WinDevices.Net;
using Xunit;

namespace WinDevicesNet.Tests;

public class DeviceInfoTests
{
    [Fact]
    public void DeviceInfo_WithManufacturerAndProduct_ShouldFormatCorrectly()
    {
        // Arrange
        var device = new DeviceInfo
        {
            Manufacturer = "TestManufacturer",
            Product = "TestProduct",
            SerialNumber = "123456"
        };

        // Act
        var result = device.ToString();

        // Assert
        result.Should().Be("TestManufacturer TestProduct (SN: 123456)");
    }

    [Fact]
    public void DeviceInfo_WithProductOnly_ShouldFormatCorrectly()
    {
        // Arrange
        var device = new DeviceInfo
        {
            Product = "TestProduct",
            SerialNumber = "123456"
        };

        // Act
        var result = device.ToString();

        // Assert
        result.Should().Be("TestProduct (SN: 123456)");
    }

    [Fact]
    public void DeviceInfo_WithSerialNumberOnly_ShouldFormatCorrectly()
    {
        // Arrange
        var device = new DeviceInfo
        {
            SerialNumber = "123456"
        };

        // Act
        var result = device.ToString();

        // Assert
        result.Should().Be("Device (SN: 123456)");
    }

    [Fact]
    public void DeviceInfo_WithNoIdentifiers_ShouldShowDeviceClass()
    {
        // Arrange
        var device = new DeviceInfo
        {
            DeviceClass = 0x08
        };

        // Act
        var result = device.ToString();

        // Assert
        result.Should().Be("Device (Class: 0x08)");
    }

    [Fact]
    public void DeviceInfo_Properties_ShouldInitializeCorrectly()
    {
        // Arrange & Act
        var device = new DeviceInfo
        {
            Manufacturer = "TestMfg",
            Product = "TestProd",
            SerialNumber = "SN123",
            Description = "Test Description",
            DeviceId = "DEV123",
            FriendlyName = "Friendly",
            DevicePath = @"\\?\usb#...",
            VendorId = 0x1234,
            ProductId = 0x5678,
            DeviceClass = 0x08,
            DeviceSubClass = 0x06,
            DeviceProtocol = 0x50,
            IsConnected = true,
            IsUsbDevice = true
        };

        // Assert
        device.Manufacturer.Should().Be("TestMfg");
        device.Product.Should().Be("TestProd");
        device.SerialNumber.Should().Be("SN123");
        device.Description.Should().Be("Test Description");
        device.DeviceId.Should().Be("DEV123");
        device.FriendlyName.Should().Be("Friendly");
        device.DevicePath.Should().Be(@"\\?\usb#...");
        device.VendorId.Should().Be(0x1234);
        device.ProductId.Should().Be(0x5678);
        device.DeviceClass.Should().Be(0x08);
        device.DeviceSubClass.Should().Be(0x06);
        device.DeviceProtocol.Should().Be(0x50);
        device.IsConnected.Should().BeTrue();
        device.IsUsbDevice.Should().BeTrue();
    }
}
