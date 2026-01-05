using System;
using System.Collections.Generic;
using System.Linq;
using WinDevices.Net;

namespace WinDevices.TestConsole
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length > 0 && args[0] == "--mass-storage")
            {
                TestUsbMassStorageEnumeration();
                return;
            }

            if (args.Length > 0 && args[0] == "--all")
            {
                ShowAllUsbDevicesWithDescriptors();
                return;
            }

            Console.WriteLine("=== USB Device Detection for Kingston DataTraveler 70 ===");
            Console.WriteLine("Target Device: Kingston DataTraveler 70");
            Console.WriteLine("Expected Serial: 1831BFBD3065F49039600403\n");

            FindTargetDevice();
        }

        static void ShowAllUsbDevicesWithDescriptors()
        {
            Console.WriteLine("=== ALL USB DEVICES WITH DESCRIPTOR INFO ===\n");

            try
            {
                using var deviceManager = new DeviceManager();
                deviceManager.EnumerateUsbDevices();

                var allDevices = deviceManager.GetAllDevices().ToList();
                Console.WriteLine($"Found {allDevices.Count} USB device(s):\n");

                int deviceNum = 1;
                foreach (var device in allDevices)
                {
                    Console.WriteLine($"========== Device #{deviceNum++} ==========");
                    Console.WriteLine($"  Manufacturer: {device.Manufacturer}");
                    Console.WriteLine($"  Product:      {device.Product}");
                    Console.WriteLine($"  Serial:       {device.SerialNumber}");
                    Console.WriteLine($"  VID:PID:      {device.VendorId:X4}:{device.ProductId:X4}");
                    Console.WriteLine($"  Connected:    {device.IsConnected}");
                    Console.WriteLine();
                    Console.WriteLine($"  ===>Device Descriptor<===");
                    Console.WriteLine($"    bDeviceClass:    0x{device.DeviceClass:X2} -> {GetUsbClassName(device.DeviceClass)}");
                    Console.WriteLine();
                    Console.WriteLine($"  ===>Interface Descriptor<===");
                    Console.WriteLine($"    bInterfaceClass: 0x{device.InterfaceClass:X2} -> {GetUsbClassName(device.InterfaceClass)}");
                    Console.WriteLine();
                    Console.WriteLine($"  Windows Device Setup Class:");
                    Console.WriteLine($"    GUID: {device.DeviceClassGuid}");
                    Console.WriteLine($"    Name: {device.DeviceClassName}");
                    Console.WriteLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }
        }

        static void TestUsbMassStorageEnumeration()
        {
            Console.WriteLine("=== USB MASS STORAGE ENUMERATION ===\n");

            try
            {
                using var deviceManager = new DeviceManager();

                // Use the new EnumerateUsbMassStorage method
                Console.WriteLine("Calling EnumerateUsbMassStorage()...\n");
                deviceManager.EnumerateUsbMassStorage();

                var massStorageDevices = deviceManager.GetAllDevices().ToList();

                Console.WriteLine($"Found {massStorageDevices.Count} USB Mass Storage device(s):\n");

                foreach (var device in massStorageDevices)
                {
                    Console.WriteLine($"  Device: {device.Manufacturer} {device.Product}".Trim());
                    Console.WriteLine($"    Serial Number: {device.SerialNumber}");
                    Console.WriteLine($"    VID:PID = {device.VendorId:X4}:{device.ProductId:X4}");
                    Console.WriteLine($"    Interface Class: 0x{device.InterfaceClass:X2} ({GetUsbClassName(device.InterfaceClass)})");
                    Console.WriteLine($"    Connected: {device.IsConnected}");
                    Console.WriteLine();
                }

                if (massStorageDevices.Count == 0)
                {
                    Console.WriteLine("  No USB mass storage devices found.");
                    Console.WriteLine("  Connect a USB flash drive or external hard drive to test.");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }
        }
        
        static void FindTargetDevice()
        {
            try
            {
                using var deviceManager = new DeviceManager();
                
                // Clear any existing devices
                deviceManager.ClearDevices();
                
                Console.WriteLine("Enumerating USB devices...");
                deviceManager.EnumerateUsbDevices();
                
                var allDevices = deviceManager.GetAllDevices().ToList();
                var connectedDevices = allDevices.Where(d => d.IsConnected).ToList();

                Console.WriteLine($"Found {allDevices.Count} total USB devices, {connectedDevices.Count} marked as connected.\n");

                // Look for Kingston DataTraveler specifically in all devices (not just connected)
                var targetSerial = "1831BFBD3065F49039600403";
                var kingstonDevices = allDevices.Where(d => 
                    (d.Manufacturer?.Contains("Kingston", StringComparison.OrdinalIgnoreCase) == true) ||
                    (d.Product?.Contains("DataTraveler", StringComparison.OrdinalIgnoreCase) == true) ||
                    (d.SerialNumber == targetSerial)
                ).ToList();

                if (kingstonDevices.Any())
                {
                    Console.WriteLine("*** FOUND KINGSTON/DATATRAVELER DEVICES ***");
                    foreach (var device in kingstonDevices)
                    {
                        PrintDeviceDetails(device);
                        
                        if (device.SerialNumber == targetSerial)
                        {
                            Console.WriteLine("*** THIS IS YOUR TARGET DEVICE! ***");
                            Console.WriteLine("*** DEVICE FOUND SUCCESSFULLY! ***\n");
                        }
                    }
                }
                else
                {
                    Console.WriteLine("No Kingston/DataTraveler devices found with USB enumeration.");
                    Console.WriteLine("Trying mass storage enumeration...\n");
                    
                    // Try mass storage enumeration
                    EnumerateMassStorageDevices(targetSerial);
                }
                
                // Show all devices for debugging if we need to see what's available
                if (!kingstonDevices.Any())
                {
                    Console.WriteLine("\n=== ALL USB DEVICES (for debugging) ===");
                    for (int i = 0; i < Math.Min(allDevices.Count, 10); i++) // Limit to first 10 to avoid spam
                    {
                        var device = allDevices[i];
                        Console.WriteLine($"#{i+1}: {device.Manufacturer} - {device.Product}");
                        Console.WriteLine($"  Serial: {device.SerialNumber}");
                        Console.WriteLine($"  Connected: {device.IsConnected}");
                        Console.WriteLine($"  Windows Class: {device.DeviceClassName}");
                        Console.WriteLine($"  VID:PID = {device.VendorId:X4}:{device.ProductId:X4}");
                        Console.WriteLine($"  ===>Device Descriptor<===");
                        Console.WriteLine($"    bDeviceClass:    0x{device.DeviceClass:X2} -> {GetUsbClassName(device.DeviceClass)}");
                        Console.WriteLine($"  ===>Interface Descriptor<===");
                        Console.WriteLine($"    bInterfaceClass: 0x{device.InterfaceClass:X2} -> {GetUsbClassName(device.InterfaceClass)}");
                        Console.WriteLine();
                    }

                    if (allDevices.Count > 10)
                    {
                        Console.WriteLine($"... and {allDevices.Count - 10} more devices");
                    }
                }

                // Show summary of interface classes found
                ShowInterfaceClassSummary(allDevices);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error during USB enumeration: {ex.Message}");
                Console.WriteLine($"Stack trace: {ex.StackTrace}");
            }
        }
        
        static void EnumerateMassStorageDevices(string targetSerial)
        {
            try
            {
                using var deviceManager = new DeviceManager();
                
                var massStorageGuids = new[]
                {
                    DeviceClassGuids.MassStorage,
                    DeviceClassGuids.UsbStorage,
                    DeviceClassGuids.DiskDrive,
                    DeviceClassGuids.PortableDevices
                };

                var foundDevices = new List<DeviceInfo>();
                
                foreach (var classGuid in massStorageGuids)
                {
                    try
                    {
                        deviceManager.ClearDevices();
                        deviceManager.EnumerateByDeviceClass(classGuid);
                        
                        var classDevices = deviceManager.GetAllDevices()
                            .Where(d => d.IsConnected)
                            .ToList();

                        Console.WriteLine($"Class {DeviceClassGuids.GetClassName(classGuid)}: {classDevices.Count} devices");
                        
                        var kingstonInClass = classDevices.Where(d => 
                            (d.Manufacturer?.Contains("Kingston", StringComparison.OrdinalIgnoreCase) == true) ||
                            (d.Product?.Contains("DataTraveler", StringComparison.OrdinalIgnoreCase) == true) ||
                            (d.SerialNumber == targetSerial)
                        ).ToList();
                        
                        if (kingstonInClass.Any())
                        {
                            Console.WriteLine("*** FOUND KINGSTON DEVICES IN THIS CLASS ***");
                            foreach (var device in kingstonInClass)
                            {
                                PrintDeviceDetails(device);
                                if (device.SerialNumber == targetSerial)
                                {
                                    Console.WriteLine("*** THIS IS YOUR TARGET DEVICE! ***");
                                }
                            }
                        }
                        
                        foundDevices.AddRange(classDevices);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error enumerating {classGuid}: {ex.Message}");
                    }
                }
                
                // Remove duplicates and show summary
                var uniqueDevices = foundDevices
                    .GroupBy(d => d.DeviceId)
                    .Select(g => g.First())
                    .ToList();
                    
                Console.WriteLine($"\n=== MASS STORAGE DEVICES SUMMARY ===");
                Console.WriteLine($"Total unique devices: {uniqueDevices.Count}");
                
                foreach (var device in uniqueDevices)
                {
                    Console.WriteLine($"• {device.Manufacturer} - {device.Product} (SN: {device.SerialNumber})");
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error during mass storage enumeration: {ex.Message}");
            }
        }
        
        static void PrintDeviceDetails(DeviceInfo device)
        {
            Console.WriteLine($"=== DEVICE DETAILS ===");
            Console.WriteLine($"Manufacturer: '{device.Manufacturer}'");
            Console.WriteLine($"Product: '{device.Product}'");
            Console.WriteLine($"SerialNumber: '{device.SerialNumber}'");
            Console.WriteLine($"Description: '{device.Description}'");
            Console.WriteLine($"FriendlyName: '{device.FriendlyName}'");
            Console.WriteLine($"DeviceId: '{device.DeviceId}'");
            Console.WriteLine($"DevicePath: '{device.DevicePath}'");
            Console.WriteLine($"Connected: {device.IsConnected}, USB: {device.IsUsbDevice}");
            Console.WriteLine($"VendorId: 0x{device.VendorId:X4}, ProductId: 0x{device.ProductId:X4}");
            Console.WriteLine($"USB DeviceClass: 0x{device.DeviceClass:X2} ({GetUsbClassName(device.DeviceClass)})");
            Console.WriteLine($"USB InterfaceClass: 0x{device.InterfaceClass:X2} ({GetUsbClassName(device.InterfaceClass)})");
            Console.WriteLine($"USB SubClass: 0x{device.DeviceSubClass:X2}, Protocol: 0x{device.DeviceProtocol:X2}");
            Console.WriteLine($"DeviceClassGuid: {device.DeviceClassGuid}");
            Console.WriteLine($"DeviceClassName: '{device.DeviceClassName}'");
            Console.WriteLine();
        }

        static string GetUsbClassName(uint classCode)
        {
            return classCode switch
            {
                0x00 => "Interface Class Defined",
                0x01 => "Audio",
                0x02 => "Communications",
                0x03 => "Human Interface Device",
                0x05 => "Physical",
                0x06 => "Image",
                0x07 => "Printer",
                0x08 => "Mass Storage",
                0x09 => "Hub",
                0x0A => "CDC-Data",
                0x0B => "Smart Card",
                0x0D => "Content Security",
                0x0E => "Video",
                0x0F => "Personal Healthcare",
                0x10 => "Audio/Video Devices",
                0x11 => "Billboard Device",
                0x12 => "USB Type-C Bridge",
                0xDC => "Diagnostic Device",
                0xE0 => "Wireless Controller",
                0xEF => "Miscellaneous",
                0xFE => "Application Specific",
                0xFF => "Vendor Specific / Not Set",
                _ => "Unknown"
            };
        }

        static void ShowInterfaceClassSummary(List<DeviceInfo> devices)
        {
            Console.WriteLine("\n=== INTERFACE CLASS SUMMARY ===");

            // Group devices by interface class
            var interfaceClassGroups = devices
                .Where(d => d.InterfaceClass != 0xFF) // Exclude "not set"
                .GroupBy(d => d.InterfaceClass)
                .OrderBy(g => g.Key)
                .ToList();

            if (!interfaceClassGroups.Any())
            {
                Console.WriteLine("No interface class information available for enumerated devices.");
                return;
            }

            Console.WriteLine($"Found {interfaceClassGroups.Count} different interface classes:\n");

            foreach (var group in interfaceClassGroups)
            {
                var className = GetUsbClassName(group.Key);
                Console.WriteLine($"Interface Class 0x{group.Key:X2} ({className}): {group.Count()} device(s)");

                // Show a few example devices for this class
                var examples = group.Take(3).ToList();
                foreach (var device in examples)
                {
                    var deviceName = !string.IsNullOrEmpty(device.Product)
                        ? $"{device.Manufacturer} {device.Product}".Trim()
                        : device.FriendlyName;

                    if (!string.IsNullOrEmpty(deviceName))
                    {
                        Console.WriteLine($"  - {deviceName}");
                    }
                }

                if (group.Count() > 3)
                {
                    Console.WriteLine($"  ... and {group.Count() - 3} more");
                }

                Console.WriteLine();
            }

            // Show count of devices without interface class
            var noInterfaceClass = devices.Count(d => d.InterfaceClass == 0xFF);
            if (noInterfaceClass > 0)
            {
                Console.WriteLine($"Note: {noInterfaceClass} device(s) have no interface class information (0xFF)");
            }
        }
        
        static void TestUsbDeviceEnumeration(DeviceManager deviceManager)
        {
            try
            {
                deviceManager.ClearDevices();
                deviceManager.EnumerateUsbDevices();
                
                var devices = deviceManager.GetAllDevices();
                Console.WriteLine($"Found {devices.Count} USB devices:");
                
                foreach (var device in devices)
                {
                    Console.WriteLine($"  ======================================");
                    Console.WriteLine($"  Manufacturer: '{device.Manufacturer}'");
                    Console.WriteLine($"  Product: '{device.Product}'");
                    Console.WriteLine($"  SerialNumber: '{device.SerialNumber}'");
                    Console.WriteLine($"  Description: '{device.Description}'");
                    Console.WriteLine($"  FriendlyName: '{device.FriendlyName}'");
                    Console.WriteLine($"  DeviceId: '{device.DeviceId}'");
                    Console.WriteLine($"  DevicePath: '{device.DevicePath}'");
                    Console.WriteLine($"  Connected: {device.IsConnected}, USB: {device.IsUsbDevice}");
                    Console.WriteLine($"  VendorId: 0x{device.VendorId:X4}, ProductId: 0x{device.ProductId:X4}");
                    Console.WriteLine($"  USB DeviceClass: 0x{device.DeviceClass:X2}, SubClass: 0x{device.DeviceSubClass:X2}, Protocol: 0x{device.DeviceProtocol:X2}");
                    Console.WriteLine($"  DeviceClassGuid: {device.DeviceClassGuid}");
                    Console.WriteLine($"  DeviceClassName: '{device.DeviceClassName}'");
                    
                    // Check if this device has useful information
                    bool hasSerialNumber = !string.IsNullOrEmpty(device.SerialNumber);
                    bool hasManufacturer = !string.IsNullOrEmpty(device.Manufacturer);
                    bool hasProduct = !string.IsNullOrEmpty(device.Product);
                    bool hasClassGuid = device.DeviceClassGuid != Guid.Empty;
                    
                    Console.WriteLine($"  Quality Check - SN:{hasSerialNumber}, Mfg:{hasManufacturer}, Prod:{hasProduct}, ClassGuid:{hasClassGuid}");
                    Console.WriteLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"USB enumeration failed: {ex.Message}");
            }
        }
        
        static void TestDeviceClassEnumeration(DeviceManager deviceManager)
        {
            try
            {
                deviceManager.ClearDevices();
                deviceManager.EnumerateAllDevices();
                
                var devices = deviceManager.GetAllDevices();
                Console.WriteLine($"Found {devices.Count} devices (all types):");
                
                foreach (var device in devices)
                {
                    Console.WriteLine($"  - {device.Manufacturer} {device.Product}");
                    Console.WriteLine($"    Class: {device.DeviceClassName} ({device.DeviceClassGuid})");
                    Console.WriteLine($"    Connected: {device.IsConnected}, USB: {device.IsUsbDevice}");
                    Console.WriteLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"All device enumeration failed: {ex.Message}");
            }
        }
        
        static void TestSpecificDeviceClasses(DeviceManager deviceManager)
        {
            var deviceClasses = new[]
            {
                (DeviceClassGuids.UsbDevice, "USB Device"),
                (DeviceClassGuids.DiskDrive, "Disk Drive"),
                (DeviceClassGuids.PortableDevices, "Portable Devices"),
                (DeviceClassGuids.MassStorage, "Mass Storage"),
                (DeviceClassGuids.UsbStorage, "USB Storage"),
                (DeviceClassGuids.Volume, "Storage Volume"),
                (DeviceClassGuids.UsbHostController, "USB Host Controller"),
                (DeviceClassGuids.HidClass, "Human Interface Device")
            };
            
            foreach (var (classGuid, className) in deviceClasses)
            {
                try
                {
                    Console.WriteLine($"\n========== {className} ({classGuid}) ==========");
                    deviceManager.ClearDevices();
                    deviceManager.EnumerateByDeviceClass(classGuid);
                    
                    var devices = deviceManager.GetAllDevices();
                    Console.WriteLine($"Found {devices.Count} devices:");
                    
                    foreach (var device in devices)
                    {
                        Console.WriteLine($"  ----------------------------------");
                        Console.WriteLine($"  Manufacturer: '{device.Manufacturer}'");
                        Console.WriteLine($"  Product: '{device.Product}'");
                        Console.WriteLine($"  SerialNumber: '{device.SerialNumber}'");
                        Console.WriteLine($"  FriendlyName: '{device.FriendlyName}'");
                        Console.WriteLine($"  Connected: {device.IsConnected}");
                        Console.WriteLine($"  DeviceClassGuid: {device.DeviceClassGuid}");
                        Console.WriteLine($"  DeviceClassName: '{device.DeviceClassName}'");
                        
                        // Show if this device would be useful for WPF
                        bool wouldShowInWpf = !string.IsNullOrEmpty(device.Manufacturer) || 
                                             !string.IsNullOrEmpty(device.Product) || 
                                             !string.IsNullOrEmpty(device.SerialNumber);
                        Console.WriteLine($"  WPF Visible: {wouldShowInWpf}");
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine($"  Error enumerating {className}: {ex.Message}");
                }
            }
        }
        
        static void TestWpfServiceSimulation(DeviceManager deviceManager)
        {
            Console.WriteLine("\n========== WPF Service Simulation ==========");
            
            try
            {
                var allDevices = new List<DeviceInfo>();

                // Define the same device class GUIDs that WPF uses
                var deviceClassGuids = new[]
                {
                    DeviceClassGuids.Media,           // Media Device
                    DeviceClassGuids.Modem,           // Modem
                    DeviceClassGuids.Ports,           // Port
                    DeviceClassGuids.UsbDevice,       // USB Device
                    DeviceClassGuids.HidClass,        // Human Interface Device
                    DeviceClassGuids.Keyboard,        // Keyboard
                    DeviceClassGuids.Mouse,           // Mouse
                    DeviceClassGuids.DiskDrive,       // Disk Drive
                    DeviceClassGuids.Display,         // Display Adapter
                    DeviceClassGuids.Net,             // Network Adapter
                    DeviceClassGuids.Bluetooth,       // Bluetooth Device
                    DeviceClassGuids.Image,           // Imaging Device
                    DeviceClassGuids.Printer,         // Printer
                    DeviceClassGuids.Battery,         // Battery
                    DeviceClassGuids.Biometric,       // Biometric Device
                    DeviceClassGuids.UsbHostController, // USB Host Controller
                    DeviceClassGuids.Volume,          // Storage Volume
                    DeviceClassGuids.System,          // System Device
                    DeviceClassGuids.PortableDevices, // Portable Devices (WPD)
                    DeviceClassGuids.MassStorage,     // Mass Storage Device
                    DeviceClassGuids.UsbStorage,      // USB Storage Device
                    DeviceClassGuids.Wpd,             // Windows Portable Device
                    DeviceClassGuids.UsbTypeC         // USB Type-C Connector
                };

                Console.WriteLine($"Enumerating {deviceClassGuids.Length} device classes...");

                // Enumerate devices for each class GUID (same as WPF does)
                foreach (var classGuid in deviceClassGuids)
                {
                    try
                    {
                        deviceManager.ClearDevices();
                        deviceManager.EnumerateByDeviceClass(classGuid);

                        var classDevices = deviceManager.GetAllDevices()
                            .Where(d => d.IsConnected)
                            .ToList();

                        if (classDevices.Count > 0)
                        {
                            Console.WriteLine($"  {DeviceClassGuids.GetClassName(classGuid)}: {classDevices.Count} devices");
                        }

                        allDevices.AddRange(classDevices);
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"  Error with {DeviceClassGuids.GetClassName(classGuid)}: {ex.Message}");
                    }
                }

                // Remove duplicates (same as WPF does)
                var uniqueDevices = allDevices
                    .GroupBy(d => d.DeviceId)
                    .Select(g => g.First())
                    .ToList();

                Console.WriteLine($"\nTotal devices found: {allDevices.Count}");
                Console.WriteLine($"Unique devices after deduplication: {uniqueDevices.Count}");
                
                Console.WriteLine("\nDevices that would appear in WPF:");
                foreach (var device in uniqueDevices)
                {
                    // This is what WPF would show
                    string manufacturer = device.Manufacturer ?? "Unknown";
                    string model = device.Product ?? "Unknown Device";
                    string serialNumber = device.SerialNumber ?? "";
                    string deviceClassName = device.DeviceClassName ?? "Unknown";
                    
                    Console.WriteLine($"  - {manufacturer} {model}");
                    Console.WriteLine($"    Serial: '{serialNumber}'");
                    Console.WriteLine($"    Class: {deviceClassName}");
                    Console.WriteLine($"    DeviceId: {device.DeviceId}");
                    Console.WriteLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"WPF simulation failed: {ex.Message}");
            }
        }
    }
}