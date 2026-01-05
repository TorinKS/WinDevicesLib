using System;

namespace WinDevices.Net;

/// <summary>
/// Windows Device Setup Class GUIDs
/// Reference: https://learn.microsoft.com/en-us/windows-hardware/drivers/install/overview-of-device-setup-classes
/// </summary>
public static class DeviceClassGuids
{
    /// <summary>
    /// Media - Sound, video and game controllers
    /// </summary>
    public static readonly Guid Media = new Guid("4d36e96c-e325-11ce-bfc1-08002be10318");

    /// <summary>
    /// Modem
    /// </summary>
    public static readonly Guid Modem = new Guid("4D36E96D-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// Ports (COM and LPT ports)
    /// </summary>
    public static readonly Guid Ports = new Guid("4D36E978-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// USB Device
    /// </summary>
    public static readonly Guid UsbDevice = new Guid("88BAE032-5A81-49f0-BC3D-A4FF138216D6");

    /// <summary>
    /// Human Interface Devices (HID)
    /// </summary>
    public static readonly Guid HidClass = new Guid("745a17a0-74d3-11d0-b6fe-00a0c90f57da");

    /// <summary>
    /// Keyboard
    /// </summary>
    public static readonly Guid Keyboard = new Guid("4D36E96B-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// Mouse
    /// </summary>
    public static readonly Guid Mouse = new Guid("4D36E96F-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// Disk drives
    /// </summary>
    public static readonly Guid DiskDrive = new Guid("4D36E967-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// Display adapters
    /// </summary>
    public static readonly Guid Display = new Guid("4D36E968-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// Network adapters
    /// </summary>
    public static readonly Guid Net = new Guid("4D36E972-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// Bluetooth Devices
    /// </summary>
    public static readonly Guid Bluetooth = new Guid("e0cbf06c-cd8b-4647-bb8a-263b43f0f974");

    /// <summary>
    /// Image - Cameras, Scanners
    /// </summary>
    public static readonly Guid Image = new Guid("6bdd1fc6-810f-11d0-bec7-08002be2092f");

    /// <summary>
    /// Printer
    /// </summary>
    public static readonly Guid Printer = new Guid("4D36E979-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// Battery Devices
    /// </summary>
    public static readonly Guid Battery = new Guid("72631e54-78a4-11d0-bcf7-00aa00b7b32a");

    /// <summary>
    /// Biometric Device
    /// </summary>
    public static readonly Guid Biometric = new Guid("53D29EF7-377C-4D14-864B-EB3A85769359");

    /// <summary>
    /// USB Host Controller
    /// </summary>
    public static readonly Guid UsbHostController = new Guid("36FC9E60-C465-11CF-8056-444553540000");

    /// <summary>
    /// Storage Volume
    /// </summary>
    public static readonly Guid Volume = new Guid("71a27cdd-812a-11d0-bec7-08002be2092f");

    /// <summary>
    /// System devices
    /// </summary>
    public static readonly Guid System = new Guid("4D36E97D-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// Portable Devices (WPD) - Modern USB devices including Type-C
    /// </summary>
    public static readonly Guid PortableDevices = new Guid("6AC27878-A6FA-4155-BA85-F98F491D4F33");

    /// <summary>
    /// Mass Storage Device
    /// </summary>
    public static readonly Guid MassStorage = new Guid("4D36E967-E325-11CE-BFC1-08002BE10318");

    /// <summary>
    /// USB Storage Device
    /// </summary>
    public static readonly Guid UsbStorage = new Guid("53F56307-B6BF-11D0-94F2-00A0C91EFB8B");

    /// <summary>
    /// Windows Portable Device (WPD)
    /// </summary>
    public static readonly Guid Wpd = new Guid("EEC5AD98-8080-425F-922A-DABF3DE3F69A");

    /// <summary>
    /// USB Type-C Connector System Software Interface
    /// </summary>
    public static readonly Guid UsbTypeC = new Guid("ad5f6d9d-4d35-4986-88d1-5c9b0a5ee0e2");

    /// <summary>
    /// Gets a friendly name for a device class GUID
    /// </summary>
    /// <param name="classGuid">The device class GUID</param>
    /// <returns>Friendly name or "Unknown" if not recognized</returns>
    public static string GetClassName(Guid classGuid)
    {
        if (classGuid == Media) return "Media Device";
        if (classGuid == Modem) return "Modem";
        if (classGuid == Ports) return "Port";
        if (classGuid == UsbDevice) return "USB Device";
        if (classGuid == HidClass) return "Human Interface Device";
        if (classGuid == Keyboard) return "Keyboard";
        if (classGuid == Mouse) return "Mouse";
        if (classGuid == DiskDrive) return "Disk Drive";
        if (classGuid == Display) return "Display Adapter";
        if (classGuid == Net) return "Network Adapter";
        if (classGuid == Bluetooth) return "Bluetooth Device";
        if (classGuid == Image) return "Imaging Device";
        if (classGuid == Printer) return "Printer";
        if (classGuid == Battery) return "Battery";
        if (classGuid == Biometric) return "Biometric Device";
        if (classGuid == UsbHostController) return "USB Host Controller";
        if (classGuid == Volume) return "Storage Volume";
        if (classGuid == System) return "System Device";
        if (classGuid == PortableDevices) return "Portable Device";
        if (classGuid == MassStorage) return "Mass Storage Device";
        if (classGuid == UsbStorage) return "USB Storage Device";
        if (classGuid == Wpd) return "Windows Portable Device";
        if (classGuid == UsbTypeC) return "USB Type-C Device";
        
        return "Unknown Device";
    }
}
