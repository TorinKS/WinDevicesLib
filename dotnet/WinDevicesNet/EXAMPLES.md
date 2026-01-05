# WinDevicesNet - Code Examples

Comprehensive examples for using the WinDevicesNet library.

## Table of Contents

- [Basic Usage](#basic-usage)
- [Device Filtering](#device-filtering)
- [Device Class Enumeration](#device-class-enumeration)
- [Real-World Scenarios](#real-world-scenarios)
- [Advanced Usage](#advanced-usage)
- [Integration Examples](#integration-examples)
- [Error Handling Examples](#error-handling-examples)

## Basic Usage

### Simple Device Listing

```csharp
using WinDevices.Net;

class Program
{
    static void Main()
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        Console.WriteLine($"Found {manager.GetDeviceCount()} USB devices:\n");

        foreach (var device in manager.GetAllDevices())
        {
            Console.WriteLine(device);
            Console.WriteLine();
        }
    }
}
```

### Detailed Device Information

```csharp
using var manager = new DeviceManager();
manager.EnumerateUsbDevices();

foreach (var device in manager.GetAllDevices())
{
    Console.WriteLine("========================================");
    Console.WriteLine($"Manufacturer:  {device.Manufacturer ?? "N/A"}");
    Console.WriteLine($"Product:       {device.Product ?? "N/A"}");
    Console.WriteLine($"Serial Number: {device.SerialNumber ?? "N/A"}");
    Console.WriteLine($"VID:PID:       {device.VendorId:X4}:{device.ProductId:X4}");
    Console.WriteLine($"Device Class:  0x{device.DeviceClass:X2}");
    Console.WriteLine($"Connected:     {device.IsConnected}");
    Console.WriteLine($"Device Path:   {device.DevicePath ?? "N/A"}");
    Console.WriteLine();
}
```

### Check Library Version

```csharp
var version = DeviceManager.GetVersion();
Console.WriteLine($"WinDevices API Version: {version}");
```

## Device Filtering

### Find Devices by Vendor

```csharp
using var manager = new DeviceManager();
manager.EnumerateUsbDevices();

// Find all Microsoft devices (VID: 0x045E)
var microsoftDevices = manager.FindDevices(d => d.VendorId == 0x045E);

Console.WriteLine($"Found {microsoftDevices.Count} Microsoft devices:");
foreach (var device in microsoftDevices)
{
    Console.WriteLine($"  {device.Product}");
}
```

### Find Devices by Product ID

```csharp
// Find specific product
var productId = 0x0407; // Example product ID
var devices = manager.FindDevices(d => d.ProductId == productId);

if (devices.Any())
{
    Console.WriteLine($"Found {devices.Count} device(s) with PID {productId:X4}:");
    foreach (var device in devices)
    {
        Console.WriteLine($"  {device.Manufacturer} {device.Product}");
    }
}
else
{
    Console.WriteLine($"No devices found with PID {productId:X4}");
}
```

### Find Devices by Class

```csharp
// Find all mass storage devices
var storageDevices = manager.FindDevices(d => d.DeviceClass == 0x08);

Console.WriteLine($"Mass Storage Devices ({storageDevices.Count}):");
foreach (var device in storageDevices)
{
    Console.WriteLine($"  {device.Product} - {device.SerialNumber}");
}

// Find all HID devices
var hidDevices = manager.FindDevices(d => d.DeviceClass == 0x03);

Console.WriteLine($"\nHID Devices ({hidDevices.Count}):");
foreach (var device in hidDevices)
{
    Console.WriteLine($"  {device.Product}");
}
```

### Complex Filters with LINQ

```csharp
// Find USB 3.0 mass storage devices with serial numbers
var usb3Storage = manager.GetAllDevices()
    .Where(d => d.DeviceClass == 0x08)
    .Where(d => !string.IsNullOrEmpty(d.SerialNumber))
    .Where(d => d.IsConnected)
    .OrderBy(d => d.Manufacturer)
    .ThenBy(d => d.Product)
    .ToList();

// Find devices from specific manufacturers
string[] targetManufacturers = { "SanDisk", "Kingston", "Samsung" };
var targetDevices = manager.GetAllDevices()
    .Where(d => targetManufacturers.Any(m =>
        d.Manufacturer?.Contains(m, StringComparison.OrdinalIgnoreCase) == true))
    .ToList();
```

## Device Class Enumeration

### Enumerate Keyboards

```csharp
using var manager = new DeviceManager();
manager.EnumerateKeyboards();

Console.WriteLine($"Found {manager.GetDeviceCount()} keyboard(s):");
foreach (var device in manager.GetAllDevices())
{
    Console.WriteLine($"  {device.FriendlyName ?? device.Product}");
}
```

### Enumerate Disk Drives

```csharp
using var manager = new DeviceManager();
manager.EnumerateDiskDrives();

Console.WriteLine($"Found {manager.GetDeviceCount()} disk drive(s):");
foreach (var device in manager.GetAllDevices())
{
    Console.WriteLine($"  {device.Product} ({device.SerialNumber})");
}
```

### Enumerate USB Mass Storage Only

```csharp
using var manager = new DeviceManager();
manager.EnumerateUsbMassStorage();

Console.WriteLine($"Found {manager.GetDeviceCount()} USB mass storage device(s):");
foreach (var device in manager.GetAllDevices())
{
    Console.WriteLine($"  {device.Product}");
    Console.WriteLine($"    VID:PID: {device.VendorId:X4}:{device.ProductId:X4}");
    Console.WriteLine($"    Serial: {device.SerialNumber}");
}
```

### Enumerate by Custom Device Class GUID

```csharp
using var manager = new DeviceManager();

// Bluetooth devices
var bluetoothGuid = new Guid("E0CBF06C-CD8B-4647-BB8A-263B43F0F974");
manager.EnumerateByDeviceClass(bluetoothGuid);

Console.WriteLine($"Found {manager.GetDeviceCount()} Bluetooth device(s)");
```

## Real-World Scenarios

### USB Security Monitor

```csharp
using System.Security.Cryptography;
using System.Text;
using WinDevices.Net;

class UsbSecurityMonitor
{
    private readonly HashSet<string> _authorizedDevices = new();
    private readonly string _authorizedDevicesFile = "authorized_devices.txt";

    public void LoadAuthorizedDevices()
    {
        if (File.Exists(_authorizedDevicesFile))
        {
            _authorizedDevices.UnionWith(File.ReadAllLines(_authorizedDevicesFile));
        }
    }

    public void ScanForUnauthorizedDevices()
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var connectedDevices = manager.GetAllDevices()
            .Where(d => d.IsConnected && !string.IsNullOrEmpty(d.SerialNumber))
            .ToList();

        foreach (var device in connectedDevices)
        {
            var deviceHash = ComputeDeviceHash(device);

            if (!_authorizedDevices.Contains(deviceHash))
            {
                LogUnauthorizedDevice(device);
                Console.WriteLine($"UNAUTHORIZED DEVICE DETECTED:");
                Console.WriteLine($"   {device.Manufacturer} {device.Product}");
                Console.WriteLine($"   Serial: {device.SerialNumber}");
                Console.WriteLine($"   VID:PID: {device.VendorId:X4}:{device.ProductId:X4}");
            }
        }
    }

    public void AuthorizeDevice(string serialNumber)
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var device = manager.FindDeviceBySerialNumber(serialNumber);
        if (device != null)
        {
            var hash = ComputeDeviceHash(device);
            _authorizedDevices.Add(hash);
            File.AppendAllLines(_authorizedDevicesFile, new[] { hash });
            Console.WriteLine($"Device authorized: {device}");
        }
    }

    private string ComputeDeviceHash(DeviceInfo device)
    {
        var data = $"{device.VendorId}:{device.ProductId}:{device.SerialNumber}";
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(data));
        return Convert.ToHexString(hash);
    }

    private void LogUnauthorizedDevice(DeviceInfo device)
    {
        var logEntry = $"{DateTime.Now:yyyy-MM-dd HH:mm:ss} - " +
                      $"Unauthorized: {device.Manufacturer} {device.Product} " +
                      $"(SN: {device.SerialNumber})";
        File.AppendAllText("security_log.txt", logEntry + Environment.NewLine);
    }
}

// Usage
var monitor = new UsbSecurityMonitor();
monitor.LoadAuthorizedDevices();
monitor.ScanForUnauthorizedDevices();
```

### USB Device Inventory

```csharp
using System.Text.Json;
using WinDevices.Net;

class DeviceInventory
{
    public class InventoryEntry
    {
        public DateTime ScanDate { get; set; }
        public string? Manufacturer { get; set; }
        public string? Product { get; set; }
        public string? SerialNumber { get; set; }
        public string VendorId { get; set; } = "";
        public string ProductId { get; set; } = "";
        public string DeviceClass { get; set; } = "";
        public bool IsConnected { get; set; }
    }

    public void PerformInventory(string outputPath)
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var inventory = manager.GetAllDevices()
            .Select(d => new InventoryEntry
            {
                ScanDate = DateTime.Now,
                Manufacturer = d.Manufacturer,
                Product = d.Product,
                SerialNumber = d.SerialNumber,
                VendorId = $"0x{d.VendorId:X4}",
                ProductId = $"0x{d.ProductId:X4}",
                DeviceClass = $"0x{d.DeviceClass:X2}",
                IsConnected = d.IsConnected
            })
            .ToList();

        var options = new JsonSerializerOptions
        {
            WriteIndented = true,
            PropertyNamingPolicy = JsonNamingPolicy.CamelCase
        };

        var json = JsonSerializer.Serialize(inventory, options);
        File.WriteAllText(outputPath, json);

        Console.WriteLine($"Inventory saved to {outputPath}");
        Console.WriteLine($"  Total devices: {inventory.Count}");
        Console.WriteLine($"  Connected: {inventory.Count(e => e.IsConnected)}");
    }

    public void GenerateCsvReport(string outputPath)
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var csv = new StringBuilder();
        csv.AppendLine("Manufacturer,Product,Serial Number,VID,PID,Device Class,Connected");

        foreach (var device in manager.GetAllDevices())
        {
            csv.AppendLine($"\"{device.Manufacturer ?? "N/A"}\"," +
                          $"\"{device.Product ?? "N/A"}\"," +
                          $"\"{device.SerialNumber ?? "N/A"}\"," +
                          $"{device.VendorId:X4}," +
                          $"{device.ProductId:X4}," +
                          $"{device.DeviceClass:X2}," +
                          $"{device.IsConnected}");
        }

        File.WriteAllText(outputPath, csv.ToString());
        Console.WriteLine($"CSV report saved to {outputPath}");
    }
}

// Usage
var inventory = new DeviceInventory();
inventory.PerformInventory("inventory.json");
inventory.GenerateCsvReport("inventory.csv");
```

### Device Change Detector

```csharp
using WinDevices.Net;

class DeviceChangeDetector
{
    private HashSet<string> _previousDevices = new();

    public void CheckForChanges()
    {
        var currentDevices = GetCurrentDeviceSignatures();

        var added = currentDevices.Except(_previousDevices).ToList();
        var removed = _previousDevices.Except(currentDevices).ToList();

        if (added.Any())
        {
            Console.WriteLine($"{added.Count} device(s) added:");
            foreach (var sig in added)
                Console.WriteLine($"   {sig}");
        }

        if (removed.Any())
        {
            Console.WriteLine($"{removed.Count} device(s) removed:");
            foreach (var sig in removed)
                Console.WriteLine($"   {sig}");
        }

        if (!added.Any() && !removed.Any())
        {
            Console.WriteLine("No changes detected");
        }

        _previousDevices = currentDevices;
    }

    private HashSet<string> GetCurrentDeviceSignatures()
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        return manager.GetAllDevices()
            .Where(d => d.IsConnected)
            .Select(d => $"{d.VendorId:X4}:{d.ProductId:X4}:{d.SerialNumber}")
            .ToHashSet();
    }
}

// Usage - poll for changes
var detector = new DeviceChangeDetector();
while (true)
{
    detector.CheckForChanges();
    Thread.Sleep(5000); // Check every 5 seconds
}
```

## Advanced Usage

### Device Search by Multiple Criteria

```csharp
public class DeviceSearchCriteria
{
    public uint? VendorId { get; set; }
    public uint? ProductId { get; set; }
    public uint? DeviceClass { get; set; }
    public string? ManufacturerContains { get; set; }
    public string? ProductContains { get; set; }
    public bool? MustHaveSerialNumber { get; set; }
}

public List<DeviceInfo> SearchDevices(DeviceSearchCriteria criteria)
{
    using var manager = new DeviceManager();
    manager.EnumerateUsbDevices();

    var query = manager.GetAllDevices().AsEnumerable();

    if (criteria.VendorId.HasValue)
        query = query.Where(d => d.VendorId == criteria.VendorId.Value);

    if (criteria.ProductId.HasValue)
        query = query.Where(d => d.ProductId == criteria.ProductId.Value);

    if (criteria.DeviceClass.HasValue)
        query = query.Where(d => d.DeviceClass == criteria.DeviceClass.Value);

    if (!string.IsNullOrEmpty(criteria.ManufacturerContains))
        query = query.Where(d => d.Manufacturer?.Contains(
            criteria.ManufacturerContains,
            StringComparison.OrdinalIgnoreCase) == true);

    if (!string.IsNullOrEmpty(criteria.ProductContains))
        query = query.Where(d => d.Product?.Contains(
            criteria.ProductContains,
            StringComparison.OrdinalIgnoreCase) == true);

    if (criteria.MustHaveSerialNumber == true)
        query = query.Where(d => !string.IsNullOrEmpty(d.SerialNumber));

    return query.ToList();
}

// Usage
var results = SearchDevices(new DeviceSearchCriteria
{
    DeviceClass = 0x08, // Mass storage
    MustHaveSerialNumber = true,
    ManufacturerContains = "SanDisk"
});
```

### Async Device Enumeration

```csharp
using WinDevices.Net;

public class AsyncDeviceService
{
    public async Task<List<DeviceInfo>> EnumerateDevicesAsync()
    {
        return await Task.Run(() =>
        {
            using var manager = new DeviceManager();
            manager.EnumerateUsbDevices();
            return manager.GetAllDevices();
        });
    }

    public async Task<DeviceInfo?> FindDeviceAsync(string serialNumber)
    {
        return await Task.Run(() =>
        {
            using var manager = new DeviceManager();
            manager.EnumerateUsbDevices();
            return manager.FindDeviceBySerialNumber(serialNumber);
        });
    }
}

// Usage
var service = new AsyncDeviceService();
var devices = await service.EnumerateDevicesAsync();
Console.WriteLine($"Found {devices.Count} devices asynchronously");
```

### Device Manager Pool

```csharp
using System.Collections.Concurrent;

public class DeviceManagerPool : IDisposable
{
    private readonly ConcurrentBag<DeviceManager> _pool = new();
    private readonly int _maxSize;

    public DeviceManagerPool(int maxSize = 5)
    {
        _maxSize = maxSize;
    }

    public DeviceManager Acquire()
    {
        if (_pool.TryTake(out var manager))
            return manager;

        return new DeviceManager();
    }

    public void Release(DeviceManager manager)
    {
        if (_pool.Count < _maxSize)
        {
            manager.ClearDevices();
            _pool.Add(manager);
        }
        else
        {
            manager.Dispose();
        }
    }

    public void Dispose()
    {
        while (_pool.TryTake(out var manager))
        {
            manager.Dispose();
        }
    }
}

// Usage
using var pool = new DeviceManagerPool();

Parallel.For(0, 10, i =>
{
    var manager = pool.Acquire();
    try
    {
        manager.EnumerateUsbDevices();
        Console.WriteLine($"Thread {i}: Found {manager.GetDeviceCount()} devices");
    }
    finally
    {
        pool.Release(manager);
    }
});
```

## Integration Examples

### ASP.NET Core Web API

```csharp
using Microsoft.AspNetCore.Mvc;
using WinDevices.Net;

[ApiController]
[Route("api/[controller]")]
public class DevicesController : ControllerBase
{
    [HttpGet]
    public ActionResult<IEnumerable<DeviceInfo>> GetDevices()
    {
        try
        {
            using var manager = new DeviceManager();
            manager.EnumerateUsbDevices();
            return Ok(manager.GetAllDevices());
        }
        catch (WinDevicesException ex)
        {
            return StatusCode(500, new { error = ex.Message, code = ex.ErrorCode });
        }
    }

    [HttpGet("{serialNumber}")]
    public ActionResult<DeviceInfo> GetDeviceBySerial(string serialNumber)
    {
        try
        {
            using var manager = new DeviceManager();
            manager.EnumerateUsbDevices();

            var device = manager.FindDeviceBySerialNumber(serialNumber);
            if (device == null)
                return NotFound(new { error = "Device not found" });

            return Ok(device);
        }
        catch (WinDevicesException ex)
        {
            return StatusCode(500, new { error = ex.Message, code = ex.ErrorCode });
        }
    }

    [HttpGet("search")]
    public ActionResult<IEnumerable<DeviceInfo>> SearchDevices(
        [FromQuery] uint? vendorId = null,
        [FromQuery] uint? deviceClass = null)
    {
        try
        {
            using var manager = new DeviceManager();
            manager.EnumerateUsbDevices();

            var devices = manager.GetAllDevices().AsEnumerable();

            if (vendorId.HasValue)
                devices = devices.Where(d => d.VendorId == vendorId.Value);

            if (deviceClass.HasValue)
                devices = devices.Where(d => d.DeviceClass == deviceClass.Value);

            return Ok(devices.ToList());
        }
        catch (WinDevicesException ex)
        {
            return StatusCode(500, new { error = ex.Message, code = ex.ErrorCode });
        }
    }

    [HttpGet("keyboards")]
    public ActionResult<IEnumerable<DeviceInfo>> GetKeyboards()
    {
        try
        {
            using var manager = new DeviceManager();
            manager.EnumerateKeyboards();
            return Ok(manager.GetAllDevices());
        }
        catch (WinDevicesException ex)
        {
            return StatusCode(500, new { error = ex.Message, code = ex.ErrorCode });
        }
    }
}
```

### Windows Service

```csharp
using WinDevices.Net;

public class UsbMonitorService : BackgroundService
{
    private readonly ILogger<UsbMonitorService> _logger;

    public UsbMonitorService(ILogger<UsbMonitorService> logger)
    {
        _logger = logger;
    }

    protected override async Task ExecuteAsync(CancellationToken stoppingToken)
    {
        _logger.LogInformation("USB Monitor Service started");

        while (!stoppingToken.IsCancellationRequested)
        {
            try
            {
                CheckDevices();
            }
            catch (Exception ex)
            {
                _logger.LogError(ex, "Error scanning USB devices");
            }

            await Task.Delay(TimeSpan.FromMinutes(5), stoppingToken);
        }

        _logger.LogInformation("USB Monitor Service stopped");
    }

    private void CheckDevices()
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var deviceCount = manager.GetDeviceCount();
        _logger.LogInformation("Device scan: {Count} USB devices found", deviceCount);

        // Check for unauthorized devices
        var unauthorizedDevices = manager.GetAllDevices()
            .Where(IsUnauthorized)
            .ToList();

        foreach (var device in unauthorizedDevices)
        {
            _logger.LogWarning("UNAUTHORIZED: {Device}", device);
        }
    }

    private bool IsUnauthorized(DeviceInfo device)
    {
        // Implement your authorization logic
        return false;
    }
}
```

### Console Application with Menu

```csharp
using WinDevices.Net;

class Program
{
    static void Main()
    {
        while (true)
        {
            Console.Clear();
            Console.WriteLine("========================================");
            Console.WriteLine("     USB Device Manager");
            Console.WriteLine("========================================");
            Console.WriteLine();
            Console.WriteLine("1. List all USB devices");
            Console.WriteLine("2. Find device by serial number");
            Console.WriteLine("3. List devices by class");
            Console.WriteLine("4. List keyboards");
            Console.WriteLine("5. List disk drives");
            Console.WriteLine("6. Export to JSON");
            Console.WriteLine("7. Show API version");
            Console.WriteLine("0. Exit");
            Console.WriteLine();
            Console.Write("Select option: ");

            var choice = Console.ReadLine();
            Console.WriteLine();

            switch (choice)
            {
                case "1": ListAllDevices(); break;
                case "2": FindBySerialNumber(); break;
                case "3": ListByClass(); break;
                case "4": ListKeyboards(); break;
                case "5": ListDiskDrives(); break;
                case "6": ExportToJson(); break;
                case "7": ShowVersion(); break;
                case "0": return;
                default: Console.WriteLine("Invalid option"); break;
            }

            Console.WriteLine("\nPress any key to continue...");
            Console.ReadKey();
        }
    }

    static void ListAllDevices()
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var devices = manager.GetAllDevices();
        Console.WriteLine($"Found {devices.Count} USB devices:\n");

        foreach (var device in devices)
        {
            Console.WriteLine(device);
            Console.WriteLine();
        }
    }

    static void FindBySerialNumber()
    {
        Console.Write("Enter serial number: ");
        var serial = Console.ReadLine();

        if (string.IsNullOrWhiteSpace(serial))
        {
            Console.WriteLine("Invalid serial number");
            return;
        }

        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var device = manager.FindDeviceBySerialNumber(serial);
        if (device != null)
        {
            Console.WriteLine("\nDevice found:");
            Console.WriteLine(device);
        }
        else
        {
            Console.WriteLine($"\nNo device found with serial number: {serial}");
        }
    }

    static void ListByClass()
    {
        Console.WriteLine("Common device classes:");
        Console.WriteLine("  0x03 - HID (Human Interface Devices)");
        Console.WriteLine("  0x08 - Mass Storage");
        Console.WriteLine("  0x0E - Video");
        Console.WriteLine("  0xEF - Miscellaneous");
        Console.Write("\nEnter device class (hex, e.g., 08): ");

        var input = Console.ReadLine();
        if (uint.TryParse(input, System.Globalization.NumberStyles.HexNumber,
            null, out var deviceClass))
        {
            using var manager = new DeviceManager();
            manager.EnumerateUsbDevices();

            var devices = manager.FindDevices(d => d.DeviceClass == deviceClass);
            Console.WriteLine($"\nFound {devices.Count} device(s) with class 0x{deviceClass:X2}:\n");

            foreach (var device in devices)
            {
                Console.WriteLine(device);
                Console.WriteLine();
            }
        }
        else
        {
            Console.WriteLine("Invalid hex number");
        }
    }

    static void ListKeyboards()
    {
        using var manager = new DeviceManager();
        manager.EnumerateKeyboards();

        Console.WriteLine($"Found {manager.GetDeviceCount()} keyboard(s):\n");
        foreach (var device in manager.GetAllDevices())
        {
            Console.WriteLine($"  {device.FriendlyName ?? device.Product}");
        }
    }

    static void ListDiskDrives()
    {
        using var manager = new DeviceManager();
        manager.EnumerateDiskDrives();

        Console.WriteLine($"Found {manager.GetDeviceCount()} disk drive(s):\n");
        foreach (var device in manager.GetAllDevices())
        {
            Console.WriteLine($"  {device.Product}");
        }
    }

    static void ExportToJson()
    {
        using var manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var json = System.Text.Json.JsonSerializer.Serialize(
            manager.GetAllDevices(),
            new System.Text.Json.JsonSerializerOptions { WriteIndented = true });

        var path = $"devices_{DateTime.Now:yyyyMMdd_HHmmss}.json";
        File.WriteAllText(path, json);
        Console.WriteLine($"Exported to {path}");
    }

    static void ShowVersion()
    {
        var version = DeviceManager.GetVersion();
        Console.WriteLine($"WinDevices API Version: {version}");
    }
}
```

## Error Handling Examples

### Comprehensive Error Handling

```csharp
public void SafeDeviceEnumeration()
{
    DeviceManager? manager = null;

    try
    {
        manager = new DeviceManager();
        manager.EnumerateUsbDevices();

        var devices = manager.GetAllDevices();
        ProcessDevices(devices);
    }
    catch (WinDevicesException ex) when (ex.ErrorCode == WinDevicesErrorCode.InvalidHandle)
    {
        Console.WriteLine("Failed to create device manager. Check DLL presence.");
    }
    catch (WinDevicesException ex) when (ex.ErrorCode == WinDevicesErrorCode.OutOfMemory)
    {
        Console.WriteLine("Out of memory during device enumeration.");
    }
    catch (WinDevicesException ex)
    {
        Console.WriteLine($"Device enumeration error: {ex.Message} (Code: {ex.ErrorCode})");
    }
    catch (ObjectDisposedException)
    {
        Console.WriteLine("Device manager was already disposed.");
    }
    catch (Exception ex)
    {
        Console.WriteLine($"Unexpected error: {ex.Message}");
    }
    finally
    {
        manager?.Dispose();
    }
}

private void ProcessDevices(List<DeviceInfo> devices)
{
    foreach (var device in devices)
    {
        Console.WriteLine(device);
    }
}
```

### Retry Pattern

```csharp
public List<DeviceInfo> EnumerateWithRetry(int maxRetries = 3)
{
    for (int attempt = 1; attempt <= maxRetries; attempt++)
    {
        try
        {
            using var manager = new DeviceManager();
            manager.EnumerateUsbDevices();
            return manager.GetAllDevices();
        }
        catch (WinDevicesException ex)
        {
            Console.WriteLine($"Attempt {attempt} failed: {ex.Message}");

            if (attempt == maxRetries)
                throw;

            Thread.Sleep(1000 * attempt); // Exponential backoff
        }
    }

    return new List<DeviceInfo>();
}
```

## More Examples

For more examples and test cases, see:
- [DeviceManagerTests.cs](../WinDevicesNet.Tests/DeviceManagerTests.cs) - Unit tests
- [IntegrationTests.cs](../WinDevicesNet.Tests/IntegrationTests.cs) - Integration tests
- [DeviceInfoTests.cs](../WinDevicesNet.Tests/DeviceInfoTests.cs) - DeviceInfo tests
- [TestConsole](../TestConsole/) - Interactive test application
