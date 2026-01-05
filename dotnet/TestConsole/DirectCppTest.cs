using System;
using System.Collections.Generic;
using System.Linq;
using WinDevices.Net;

namespace WinDevices.TestConsole
{
    class DirectCppTest
    {
        public static void Run()
        {
            Console.WriteLine("=== Direct C++ USB Test ===");
            TestDirectUsbAccess();
        }

        static void TestDirectUsbAccess()
        {
            try
            {
                using var deviceManager = new DeviceManager();

                Console.WriteLine("\n1. Testing device count after USB enumeration:");
                deviceManager.ClearDevices();
                
                // Get count before enumeration
                int countBefore = deviceManager.GetDeviceCount();
                Console.WriteLine($"Device count BEFORE enumeration: {countBefore}");
                
                // Enumerate USB devices
                Console.WriteLine("Calling USB enumeration...");
                deviceManager.EnumerateUsbDevices();
                
                // Get count after enumeration  
                int countAfter = deviceManager.GetDeviceCount();
                Console.WriteLine($"Device count AFTER enumeration: {countAfter}");
                
                if (countAfter > 0)
                {
                    Console.WriteLine("\nDevice details:");
                    for (int i = 0; i < Math.Min(countAfter, 10); i++)
                    {
                        try
                        {
                            var device = deviceManager.GetDeviceInfo(i);
                            Console.WriteLine($"  [{i}] {device.Manufacturer} {device.Product}");
                            Console.WriteLine($"      Serial: '{device.SerialNumber}'");
                            Console.WriteLine($"      Class: {device.DeviceClassName}");
                            Console.WriteLine();
                        }
                        catch (Exception ex)
                        {
                            Console.WriteLine($"  [{i}] Error getting device: {ex.Message}");
                        }
                    }
                }
                else
                {
                    Console.WriteLine("No devices found after enumeration!");
                }

                Console.WriteLine("\n2. Testing GetAllDevices method:");
                var allDevices = deviceManager.GetAllDevices();
                Console.WriteLine($"GetAllDevices returned {allDevices.Count} devices");

                foreach (var device in allDevices.Take(5))
                {
                    Console.WriteLine($"  - {device.Manufacturer} {device.Product} (SN: {device.SerialNumber})");
                }

            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
                Console.WriteLine($"Stack trace: {ex.StackTrace}");
            }
        }
    }
}