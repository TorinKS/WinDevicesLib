using System;
using System.Collections.Generic;
using System.Linq;
using WinDevices.Net;

namespace WinDevices.TestConsole
{
    class MassStorageTest
    {
        public static void Run()
        {
            Console.WriteLine("=== USB Mass Storage Device Test ===");
            TestUsbEnumeration();
            TestMassStorageDevices();
        }

        static void TestUsbEnumeration()
        {
            Console.WriteLine("\n=== Testing Original USB Enumeration ===");
            try
            {
                using var deviceManager = new DeviceManager();
                
                // Test original USB enumeration
                deviceManager.ClearDevices();
                deviceManager.EnumerateUsbDevices();
                
                var usbDevices = deviceManager.GetAllDevices()
                    .Where(d => d.IsConnected)
                    .ToList();

                Console.WriteLine($"Found {usbDevices.Count} USB devices with original method:");

                foreach (var device in usbDevices)
                {
                    Console.WriteLine($"  Device:");
                    Console.WriteLine($"    Manufacturer: '{device.Manufacturer}'");
                    Console.WriteLine($"    Product: '{device.Product}'");
                    Console.WriteLine($"    Serial: '{device.SerialNumber}'");
                    Console.WriteLine($"    DeviceId: '{device.DeviceId}'");
                    Console.WriteLine($"    FriendlyName: '{device.FriendlyName}'");
                    Console.WriteLine($"    Class: '{device.DeviceClassName}'");
                    Console.WriteLine($"    ClassGuid: {device.DeviceClassGuid}");
                    Console.WriteLine($"    VendorId: 0x{device.VendorId:X4}");
                    Console.WriteLine($"    ProductId: 0x{device.ProductId:X4}");
                    Console.WriteLine($"    Connected: {device.IsConnected}, USB: {device.IsUsbDevice}");
                    Console.WriteLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"USB enumeration error: {ex.Message}");
            }
        }

        static void TestMassStorageDevices()
        {
            try
            {
                using var deviceManager = new DeviceManager();
                var massStorageDevices = new List<DeviceInfo>();

                // Only enumerate mass storage related device classes
                var massStorageGuids = new[]
                {
                    DeviceClassGuids.MassStorage,     // Mass Storage Device
                    DeviceClassGuids.UsbStorage,      // USB Storage Device  
                    DeviceClassGuids.PortableDevices, // Portable Devices (WPD)
                    DeviceClassGuids.Wpd,             // Windows Portable Device
                    DeviceClassGuids.DiskDrive        // Disk Drive
                };

                Console.WriteLine("Enumerating mass storage device classes...\n");

                foreach (var classGuid in massStorageGuids)
                {
                    try
                    {
                        deviceManager.ClearDevices();
                        deviceManager.EnumerateByDeviceClass(classGuid);
                        
                        var classDevices = deviceManager.GetAllDevices()
                            .Where(d => d.IsConnected)
                            .ToList();

                        Console.WriteLine($"Class: {DeviceClassGuids.GetClassName(classGuid)}");
                        Console.WriteLine($"GUID: {classGuid}");
                        Console.WriteLine($"Found {classDevices.Count} devices");

                        foreach (var device in classDevices)
                        {
                            Console.WriteLine($"  - {device.Manufacturer} {device.Product} (SN: {device.SerialNumber})");
                            Console.WriteLine($"    Class: {device.DeviceClassName}");
                            Console.WriteLine($"    DeviceId: {device.DeviceId}");
                            Console.WriteLine($"    FriendlyName: {device.FriendlyName}");
                            Console.WriteLine();
                        }

                        massStorageDevices.AddRange(classDevices);
                        Console.WriteLine("---\n");
                    }
                    catch (Exception ex)
                    {
                        Console.WriteLine($"Error with class {classGuid}: {ex.Message}\n");
                    }
                }

                // Remove duplicates
                var uniqueDevices = massStorageDevices
                    .GroupBy(d => d.DeviceId)
                    .Select(g => g.First())
                    .ToList();

                Console.WriteLine($"\n=== SUMMARY ===");
                Console.WriteLine($"Total unique mass storage devices: {uniqueDevices.Count}");
                
                foreach (var device in uniqueDevices)
                {
                    Console.WriteLine($"• {device.Manufacturer} {device.Product}");
                    Console.WriteLine($"  Class: {device.DeviceClassName}");
                    Console.WriteLine($"  Serial: {device.SerialNumber}");
                    Console.WriteLine();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }
        }
    }
}