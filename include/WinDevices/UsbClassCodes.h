#pragma once

// USB-IF Class Codes
// Reference: https://www.usb.org/defined-class-codes
// Reference: https://learn.microsoft.com/en-us/windows-hardware/drivers/usbcon/supported-usb-classes

namespace KDM
{
    // USB Specification Limits
    // Reference: USB 3.2 Specification, Section 9.6
    namespace UsbLimits
    {
        // Maximum number of endpoints per interface (USB spec allows 30 per direction)
        constexpr size_t MaxEndpointsPerDevice = 30;

        // Maximum number of ports per USB hub
        // USB 2.0: 127 devices max on a bus, practical hub limit is 7-15 ports
        // USB 3.0: Spec allows up to 15 ports per hub
        constexpr ULONG MaxPortsPerHub = 255;  // Conservative maximum for validation

        // Maximum USB string descriptor length (255 bytes per USB spec)
        constexpr size_t MaxStringDescriptorSize = 255;

        // Typical USB device count for vector pre-allocation
        constexpr size_t TypicalDeviceCount = 32;
    }

    // USB Base Class Codes (bDeviceClass / bInterfaceClass)
    namespace UsbClass
    {
        // Use class information in the Interface Descriptors
        constexpr UCHAR InterfaceClassDefined = 0x00;

        // Audio devices (speakers, microphones, sound cards)
        // Windows Device Setup Class: Media {4d36e96c-e325-11ce-bfc1-08002be10318}
        // Driver: Usbaudio.sys
        constexpr UCHAR Audio = 0x01;

        // Communications and CDC Control (modems, network adapters, serial ports)
        // Windows Device Setup Class: Ports/Modem/Net depending on subclass
        // Subclass 02h (ACM): Modem - Driver: Usbser.sys
        // Subclass 0Dh (NCM): Net - Driver: UsbNcm.sys
        // Subclass 0Eh (MBIM): Net - Driver: wmbclass.sys
        constexpr UCHAR CdcControl = 0x02;

        // Human Interface Device (keyboards, mice, game controllers)
        // Windows Device Setup Class: HIDClass {745a17a0-74d3-11d0-b6fe-00a0c90f57da}
        // Driver: Hidclass.sys, Hidusb.sys
        constexpr UCHAR Hid = 0x03;

        // Physical devices (force feedback, physical interface devices)
        // No Microsoft driver - Recommended: WinUSB
        constexpr UCHAR Physical = 0x05;

        // Image devices (cameras, scanners - Still Image Capture)
        // Windows Device Setup Class: Image {6bdd1fc6-810f-11d0-bec7-08002be2092f}
        // Driver: Usbscan.sys
        constexpr UCHAR Image = 0x06;

        // Printer devices
        // Windows Device Setup Class: Printer {4d36e979-e325-11ce-bfc1-08002be10318}
        // Driver: Usbprint.sys
        constexpr UCHAR Printer = 0x07;

        // Mass Storage devices (USB flash drives, external hard drives, card readers)
        // Windows Device Setup Class: USB or SCSIAdapter
        // Driver: Usbstor.sys, Uaspstor.sys (for UASP/SuperSpeed)
        constexpr UCHAR MassStorage = 0x08;

        // USB Hub devices
        // Windows Device Setup Class: USB {36fc9e60-c465-11cf-8056-444553540000}
        // Driver: Usbhub.sys, Usbhub3.sys (USB 3.0)
        constexpr UCHAR Hub = 0x09;

        // CDC-Data (used with CDC Control class)
        // No Microsoft driver - Recommended: WinUSB
        constexpr UCHAR CdcData = 0x0A;

        // Smart Card readers
        // Windows Device Setup Class: SmartCardReader {50dd5230-ba8a-11d1-bf5d-0000f805f530}
        // Driver: Usbccid.sys (obsolete), WUDFUsbccidDriver.dll (UMDF)
        constexpr UCHAR SmartCard = 0x0B;

        // Content Security devices
        // No Microsoft driver - Some functionality in Usbccgp.sys
        constexpr UCHAR ContentSecurity = 0x0D;

        // Video devices (webcams, video capture)
        // Windows Device Setup Class: Image {6bdd1fc6-810f-11d0-bec7-08002be2092f}
        // Driver: Usbvideo.sys
        constexpr UCHAR Video = 0x0E;

        // Personal Healthcare devices
        // No Microsoft driver - Recommended: WinUSB
        constexpr UCHAR PersonalHealthcare = 0x0F;

        // Audio/Video devices (webcams with audio)
        // No dedicated Microsoft driver
        constexpr UCHAR AudioVideo = 0x10;

        // Billboard Device Class
        constexpr UCHAR Billboard = 0x11;

        // USB Type-C Bridge Class
        constexpr UCHAR TypeCBridge = 0x12;

        // USB Bulk Display Protocol Device Class
        constexpr UCHAR BulkDisplay = 0x13;

        // MCTP over USB Protocol Endpoint Device Class
        constexpr UCHAR Mctp = 0x14;

        // I3C Device Class
        constexpr UCHAR I3C = 0x3C;

        // Diagnostic Device
        // No Microsoft driver - Recommended: WinUSB
        constexpr UCHAR Diagnostic = 0xDC;

        // Wireless Controller (Bluetooth, UWB, etc.)
        // Subclass 01h, Protocol 01h: Bluetooth
        // Windows Device Setup Class: Bluetooth {e0cbf06c-cd8b-4647-bb8a-263b43f0f974}
        // Driver: Bthusb.sys
        constexpr UCHAR WirelessController = 0xE0;

        // Miscellaneous (includes RNDIS network devices)
        // Subclass 04h, Protocol 01h: RNDIS
        // Windows Device Setup Class: Net {4d36e972-e325-11ce-bfc1-08002be10318}
        // Driver: Rndismp.sys
        constexpr UCHAR Miscellaneous = 0xEF;

        // Application Specific
        // No Microsoft driver - Recommended: WinUSB
        constexpr UCHAR ApplicationSpecific = 0xFE;

        // Vendor Specific
        // No Microsoft driver - Recommended: WinUSB
        constexpr UCHAR VendorSpecific = 0xFF;
    }

    // Mass Storage Subclass codes
    namespace MassStorageSubclass
    {
        constexpr UCHAR Scsi = 0x06;         // SCSI transparent command set
        constexpr UCHAR Rbc = 0x01;          // Reduced Block Commands
        constexpr UCHAR Mmc5 = 0x02;         // MMC-5 (ATAPI)
        constexpr UCHAR Qic157 = 0x03;       // QIC-157
        constexpr UCHAR Ufi = 0x04;          // UFI (Floppy)
        constexpr UCHAR Sff8070i = 0x05;     // SFF-8070i
        constexpr UCHAR VendorSpecific = 0xFF;
    }

    // Mass Storage Protocol codes
    namespace MassStorageProtocol
    {
        constexpr UCHAR Cbi = 0x00;          // Control/Bulk/Interrupt with command completion interrupt
        constexpr UCHAR CbiNoInt = 0x01;     // Control/Bulk/Interrupt without command completion interrupt
        constexpr UCHAR BulkOnly = 0x50;     // Bulk-Only Transport (most common)
        constexpr UCHAR Uas = 0x62;          // USB Attached SCSI (UASP) - SuperSpeed
        constexpr UCHAR VendorSpecific = 0xFF;
    }

    // Helper function to check if a USB class is a data transfer risk for DLP
    inline bool IsDataTransferClass(UCHAR usbClass)
    {
        switch (usbClass)
        {
        case UsbClass::MassStorage:      // USB drives, external HDDs
        case UsbClass::CdcControl:       // Could be network/serial
        case UsbClass::CdcData:          // Data channel
        case UsbClass::Image:            // Cameras can transfer files
        case UsbClass::Printer:          // Can receive data
        case UsbClass::WirelessController: // Bluetooth can transfer files
            return true;
        default:
            return false;
        }
    }

    // Helper function to check if a USB class is specifically mass storage
    inline bool IsMassStorageClass(UCHAR usbClass)
    {
        return usbClass == UsbClass::MassStorage;
    }
}
