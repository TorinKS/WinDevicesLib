using System;
using System.Runtime.InteropServices;

namespace WinDevices.Net.Interop;

/// <summary>
/// P/Invoke declarations for the WinDevices C API
/// </summary>
internal static class NativeMethods
{
    private const string DllName = "WinDevices.dll";

    #region Error Codes

    internal enum WdResult
    {
        Success = 0,
        InvalidHandle = -1,
        OutOfMemory = -2,
        NoDevices = -3,
        EnumFailed = -4,
        InvalidIndex = -5,
        NullPointer = -6,
        Unknown = -99
    }

    #endregion

    #region Structures

    [StructLayout(LayoutKind.Sequential)]
    public struct WdGuid
    {
        public uint Data1;
        public ushort Data2;
        public ushort Data3;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
        public byte[] Data4;

        public WdGuid(Guid guid)
        {
            var bytes = guid.ToByteArray();
            Data1 = BitConverter.ToUInt32(bytes, 0);
            Data2 = BitConverter.ToUInt16(bytes, 4);
            Data3 = BitConverter.ToUInt16(bytes, 6);
            Data4 = new byte[8];
            Array.Copy(bytes, 8, Data4, 0, 8);
        }

        public Guid ToGuid()
        {
            var bytes = new byte[16];
            BitConverter.GetBytes(Data1).CopyTo(bytes, 0);
            BitConverter.GetBytes(Data2).CopyTo(bytes, 4);
            BitConverter.GetBytes(Data3).CopyTo(bytes, 6);
            Data4.CopyTo(bytes, 8);
            return new Guid(bytes);
        }
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct WdDeviceInfo
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Manufacturer;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Product;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string SerialNumber;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string Description;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 512)]
        public string DeviceId;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string FriendlyName;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 512)]
        public string DevicePath;

        public uint VendorId;
        public uint ProductId;
        public uint DeviceClass;
        public uint InterfaceClass;
        public uint DeviceSubClass;
        public uint DeviceProtocol;

        [MarshalAs(UnmanagedType.I4)]
        public int IsConnected;

        [MarshalAs(UnmanagedType.I4)]
        public int IsUsbDevice;

        public WdGuid DeviceClassGuid;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string VendorName;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string ProductName;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string InterfaceClassName;
    }

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct WdVersionInfo
    {
        public int Major;
        public int Minor;
        public int Patch;
        public IntPtr BuildDate; // const char* - will need to marshal
    }

    #endregion

    #region Device Manager Functions

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_CreateDeviceManager(out IntPtr handle);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_DestroyDeviceManager(IntPtr handle);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_EnumerateUsbDevices(IntPtr handle);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_EnumerateAllDevices(IntPtr handle);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_EnumerateByDeviceClass(IntPtr handle, ref WdGuid classGuid);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_EnumerateUsbMassStorage(IntPtr handle);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_GetDeviceCount(IntPtr handle, out int count);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_GetDeviceInfo(IntPtr handle, int index, out WdDeviceInfo info);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_ClearDevices(IntPtr handle);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
    public static extern WdResult WD_GetVersion(out WdVersionInfo versionInfo);

    [DllImport(DllName, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    public static extern IntPtr WD_GetErrorMessage(WdResult errorCode);

    #endregion
}
