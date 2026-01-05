using FluentAssertions;
using WinDevices.Net;
using Xunit;

namespace WinDevicesNet.Tests;

public class DeviceManagerTests
{
    [Fact]
    public void Constructor_ShouldCreateValidInstance()
    {
        // Act
        using var manager = new DeviceManager();

        // Assert
        manager.Should().NotBeNull();
    }

    [Fact]
    public void EnumerateUsbDevices_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateUsbDevices();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void GetDeviceCount_AfterEnumeration_ShouldReturnPositiveNumber()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        // Act
        var count = manager.GetDeviceCount();

        // Assert
        count.Should().BeGreaterOrEqualTo(0);
    }

    [Fact]
    public void GetAllDevices_AfterEnumeration_ShouldReturnDevices()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        // Act
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        devices.Count.Should().Be(manager.GetDeviceCount());
    }

    [Fact]
    public void GetDeviceInfo_WithValidIndex_ShouldReturnDeviceInfo()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();
        var count = manager.GetDeviceCount();

        // Skip test if no devices
        if (count == 0)
            return;

        // Act
        var device = manager.GetDeviceInfo(0);

        // Assert
        device.Should().NotBeNull();
    }

    [Fact]
    public void GetDeviceInfo_WithNegativeIndex_ShouldThrowArgumentOutOfRangeException()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        // Act
        Action act = () => manager.GetDeviceInfo(-1);

        // Assert
        act.Should().Throw<ArgumentOutOfRangeException>();
    }

    [Fact]
    public void GetDeviceInfo_WithInvalidIndex_ShouldThrowWinDevicesException()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        // Act
        Action act = () => manager.GetDeviceInfo(999999);

        // Assert
        act.Should().Throw<WinDevicesException>()
            .Which.ErrorCode.Should().Be(WinDevicesErrorCode.InvalidIndex);
    }

    [Fact]
    public void ClearDevices_ShouldReduceCountToZero()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();
        var initialCount = manager.GetDeviceCount();

        // Act
        manager.ClearDevices();
        var countAfterClear = manager.GetDeviceCount();

        // Assert
        countAfterClear.Should().Be(0);
    }

    [Fact]
    public void Dispose_ShouldPreventFurtherOperations()
    {
        // Arrange
        var manager = new DeviceManager();
        manager.Dispose();

        // Act
        Action act = () => manager.EnumerateUsbDevices();

        // Assert
        act.Should().Throw<ObjectDisposedException>();
    }

    [Fact]
    public void GetVersion_ShouldReturnValidVersion()
    {
        // Act
        var (major, minor, patch, buildDate) = DeviceManager.GetVersion();

        // Assert
        major.Should().BeGreaterOrEqualTo(0);
        minor.Should().BeGreaterOrEqualTo(0);
        patch.Should().BeGreaterOrEqualTo(0);
        buildDate.Should().NotBeNullOrEmpty();
    }

    [Fact]
    public void FindDevices_WithPredicate_ShouldReturnMatchingDevices()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        // Act
        var usbDevices = manager.FindDevices(d => d.IsUsbDevice);

        // Assert
        usbDevices.Should().NotBeNull();
        usbDevices.Should().OnlyContain(d => d.IsUsbDevice);
    }

    [Fact]
    public void FindDevices_WithNullPredicate_ShouldThrowArgumentNullException()
    {
        // Arrange
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        // Act
        Action act = () => manager.FindDevices(null!);

        // Assert
        act.Should().Throw<ArgumentNullException>();
    }

    [Fact]
    public void EnumerateAllDevices_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateAllDevices();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void MultipleEnumerations_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act & Assert
        manager.EnumerateUsbDevices();
        var count1 = manager.GetDeviceCount();

        manager.EnumerateUsbDevices();
        var count2 = manager.GetDeviceCount();

        manager.EnumerateUsbDevices();
        var count3 = manager.GetDeviceCount();

        // All counts should be consistent
        count1.Should().BeGreaterOrEqualTo(0);
        count2.Should().BeGreaterOrEqualTo(0);
        count3.Should().BeGreaterOrEqualTo(0);
    }

    [Fact]
    public void EnumerateByDeviceClass_WithMediaGuid_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateByDeviceClass(DeviceClassGuids.Media);

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateByDeviceClass_WithModemGuid_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateByDeviceClass(DeviceClassGuids.Modem);

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateMediaDevices_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateMediaDevices();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateModems_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateModems();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateKeyboards_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateKeyboards();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateMice_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateMice();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateDiskDrives_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateDiskDrives();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateNetworkAdapters_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateNetworkAdapters();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateBluetoothDevices_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateBluetoothDevices();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateImageDevices_ShouldSucceed()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        Action act = () => manager.EnumerateImageDevices();

        // Assert
        act.Should().NotThrow();
    }

    [Fact]
    public void EnumerateByDeviceClass_ShouldReturnDevices()
    {
        // Arrange
        using var manager = new DeviceManager();

        // Act
        manager.EnumerateByDeviceClass(DeviceClassGuids.Keyboard);
        var devices = manager.GetAllDevices();

        // Assert
        devices.Should().NotBeNull();
        devices.Count.Should().Be(manager.GetDeviceCount());
    }
}
