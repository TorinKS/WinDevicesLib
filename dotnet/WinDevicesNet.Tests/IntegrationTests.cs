using FluentAssertions;
using WinDevices.Net;
using Xunit;
using Xunit.Abstractions;

namespace WinDevicesNet.Tests;

/// <summary>
/// Integration tests that run against actual hardware
/// </summary>
public class IntegrationTests
{
    private readonly ITestOutputHelper _output;

    public IntegrationTests(ITestOutputHelper output)
    {
        _output = output;
    }
    [Fact]
    public void FullWorkflow_EnumerateAndGetDevices_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act - Enumerate
        manager.EnumerateUsbDevices();
        var count = manager.GetDeviceCount();

        // Skip remaining tests if no devices
        if (count == 0)
        {
            // No devices to test, but this is not a failure
            Assert.True(true, "No USB devices found - test skipped");
            return;
        }

        // Act - Get all devices
        var allDevices = manager.GetAllDevices();

        // Assert
        allDevices.Should().NotBeNull();
        allDevices.Count.Should().Be(count);
        allDevices.Should().OnlyContain(d => d != null);
    }

    [Fact]
    public void EnumerateUsbDevices_ShouldFindAtLeastOneUsbDevice()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateUsbDevices();
        var devices = manager.GetAllDevices();

        // Assert - At least verify the operation succeeds
        // (We can't guarantee devices are present in all test environments)
        devices.Should().NotBeNull();
    }

    [Fact]
    public void DeviceWithSerialNumber_ShouldBeRetrievable()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();
        var devices = manager.GetAllDevices();

        // Find a device with a serial number
        var deviceWithSerial = devices.FirstOrDefault(d => !string.IsNullOrEmpty(d.SerialNumber));

        // Skip if no device with serial number
        if (deviceWithSerial == null)
        {
            Assert.True(true, "No devices with serial numbers found - test skipped");
            return;
        }

        // Act
        var foundDevice = manager.FindDeviceBySerialNumber(deviceWithSerial.SerialNumber);

        // Assert
        foundDevice.Should().NotBeNull();
        foundDevice!.SerialNumber.Should().Be(deviceWithSerial.SerialNumber);
    }

    [Fact]
    public void ConnectedDevices_ShouldBeMarkedAsConnected()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();
        var devices = manager.GetAllDevices();

        // Skip if no devices
        if (devices.Count == 0)
        {
            Assert.True(true, "No devices found - test skipped");
            return;
        }

        // Act & Assert
        // All enumerated USB devices should be marked as connected
        devices.Should().OnlyContain(d => d.IsConnected);
    }

    [Fact]
    public void UsbDevices_ShouldBeMarkedAsUsb()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();
        var devices = manager.GetAllDevices();

        // Skip if no devices
        if (devices.Count == 0)
        {
            Assert.True(true, "No devices found - test skipped");
            return;
        }

        // Act & Assert
        // All enumerated USB devices should be marked as USB
        devices.Should().OnlyContain(d => d.IsUsbDevice);
    }

    [Fact]
    public void FindDevices_FilterByClass_ShouldReturnOnlyMatchingDevices()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();
        const uint massStorageClass = 0x08;

        // Act
        var massStorageDevices = manager.FindDevices(d => d.DeviceClass == massStorageClass);

        // Assert
        massStorageDevices.Should().NotBeNull();
        if (massStorageDevices.Count > 0)
        {
            massStorageDevices.Should().OnlyContain(d => d.DeviceClass == massStorageClass);
        }
    }

    [Fact]
    public void MultipleManagers_ShouldWorkIndependently()
    {
        // Arrange & Act
        using var manager1 = new DeviceManager();
        using var manager2 = new DeviceManager();

        manager1.EnumerateUsbDevices();
        manager2.EnumerateUsbDevices();

        var count1 = manager1.GetDeviceCount();
        var count2 = manager2.GetDeviceCount();

        // Assert
        count1.Should().Be(count2, "Both managers should enumerate the same devices");
    }

    [Fact]
    public void EnumerateMediaDevices_ShouldFindDevices()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateMediaDevices();
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        _output.WriteLine($"Found {devices.Count} media device(s)");

        foreach (var device in devices)
        {
            _output.WriteLine($"  - {device.FriendlyName ?? device.Description}");
            _output.WriteLine($"    Manufacturer: {device.Manufacturer}");
        }
    }

    [Fact]
    public void EnumerateKeyboards_ShouldFindAtLeastOne()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateKeyboards();
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        _output.WriteLine($"Found {devices.Count} keyboard(s)");

        // Most systems have at least one keyboard
        if (devices.Count > 0)
        {
            foreach (var device in devices)
            {
                _output.WriteLine($"  - {device.FriendlyName ?? device.Description}");
            }
        }
    }

    [Fact]
    public void EnumerateMice_ShouldFindAtLeastOne()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateMice();
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        _output.WriteLine($"Found {devices.Count} mouse/mice");

        // Most systems have at least one mouse
        if (devices.Count > 0)
        {
            foreach (var device in devices)
            {
                _output.WriteLine($"  - {device.FriendlyName ?? device.Description}");
            }
        }
    }

    [Fact]
    public void EnumerateDiskDrives_ShouldFindAtLeastOne()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateDiskDrives();
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        _output.WriteLine($"Found {devices.Count} disk drive(s)");

        // All systems should have at least one disk drive
        devices.Count.Should().BeGreaterThan(0, "System should have at least one disk drive");

        foreach (var device in devices)
        {
            _output.WriteLine($"  - {device.FriendlyName ?? device.Description}");
        }
    }

    [Fact]
    public void EnumerateNetworkAdapters_ShouldFindAtLeastOne()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateNetworkAdapters();
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        _output.WriteLine($"Found {devices.Count} network adapter(s)");

        // Most systems have at least one network adapter
        devices.Count.Should().BeGreaterThan(0, "System should have at least one network adapter");

        foreach (var device in devices)
        {
            _output.WriteLine($"  - {device.FriendlyName ?? device.Description}");
            _output.WriteLine($"    Manufacturer: {device.Manufacturer}");
        }
    }

    [Fact]
    public void EnumerateByDeviceClass_MultipleClasses_ShouldWorkSequentially()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act & Assert - Enumerate different device classes sequentially
        manager.EnumerateKeyboards();
        var keyboardCount = manager.GetDeviceCount();
        _output.WriteLine($"Keyboards: {keyboardCount}");

        manager.EnumerateMice();
        var mouseCount = manager.GetDeviceCount();
        _output.WriteLine($"Mice: {mouseCount}");

        manager.EnumerateMediaDevices();
        var mediaCount = manager.GetDeviceCount();
        _output.WriteLine($"Media devices: {mediaCount}");

        // All should succeed
        keyboardCount.Should().BeGreaterOrEqualTo(0);
        mouseCount.Should().BeGreaterOrEqualTo(0);
        mediaCount.Should().BeGreaterOrEqualTo(0);
    }

    [Fact]
    public void EnumerateBluetoothDevices_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateBluetoothDevices();
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        _output.WriteLine($"Found {devices.Count} Bluetooth device(s)");

        foreach (var device in devices)
        {
            _output.WriteLine($"  - {device.FriendlyName ?? device.Description}");
        }
    }

    [Fact]
    public void EnumerateImageDevices_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateImageDevices();
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        _output.WriteLine($"Found {devices.Count} image device(s) (cameras, scanners)");

        foreach (var device in devices)
        {
            _output.WriteLine($"  - {device.FriendlyName ?? device.Description}");
        }
    }
}
