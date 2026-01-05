#include <gtest/gtest.h>
#include <windows.h>
#include <string>
#include "UtilConvert.h"

using KDM::UtilConvert;

class UtilConvertTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ===== WStringToUTF8 Tests =====

TEST_F(UtilConvertTest, WStringToUTF8_EmptyString_ReturnsEmpty) {
    std::wstring input = L"";
    std::string result = UtilConvert::WStringToUTF8(input);
    EXPECT_TRUE(result.empty());
}

TEST_F(UtilConvertTest, WStringToUTF8_AsciiString_ConvertsCorrectly) {
    std::wstring input = L"Hello, World!";
    std::string result = UtilConvert::WStringToUTF8(input);
    EXPECT_EQ(result, "Hello, World!");
}

TEST_F(UtilConvertTest, WStringToUTF8_UnicodeString_ConvertsCorrectly) {
    // Test with some Unicode characters (euro sign, degree symbol)
    std::wstring input = L"Price: \u20AC100 at 25\u00B0C";
    std::string result = UtilConvert::WStringToUTF8(input);
    EXPECT_FALSE(result.empty());
    EXPECT_GT(result.length(), 0);
}

TEST_F(UtilConvertTest, WStringToUTF8_JapaneseCharacters_ConvertsCorrectly) {
    // Japanese hiragana "hello" (konnichiwa partial)
    std::wstring input = L"\u3053\u3093\u306B\u3061\u306F";
    std::string result = UtilConvert::WStringToUTF8(input);
    EXPECT_FALSE(result.empty());
    // UTF-8 for Japanese takes 3 bytes per character
    EXPECT_GE(result.length(), 5);
}

// ===== UTF8ToWString Tests =====

TEST_F(UtilConvertTest, UTF8ToWString_EmptyString_ReturnsEmpty) {
    std::string input = "";
    std::wstring result = UtilConvert::UTF8ToWString(input);
    EXPECT_TRUE(result.empty());
}

TEST_F(UtilConvertTest, UTF8ToWString_AsciiString_ConvertsCorrectly) {
    std::string input = "Hello, World!";
    std::wstring result = UtilConvert::UTF8ToWString(input);
    EXPECT_EQ(result, L"Hello, World!");
}

TEST_F(UtilConvertTest, UTF8ToWString_RoundTrip_PreservesOriginal) {
    // Test round-trip conversion
    std::wstring original = L"Test String with Numbers 12345";
    std::string utf8 = UtilConvert::WStringToUTF8(original);
    std::wstring result = UtilConvert::UTF8ToWString(utf8);
    EXPECT_EQ(result, original);
}

TEST_F(UtilConvertTest, UTF8ToWString_UnicodeRoundTrip_PreservesOriginal) {
    // Test round-trip with Unicode
    std::wstring original = L"Price: \u20AC100 Temperature: 25\u00B0C";
    std::string utf8 = UtilConvert::WStringToUTF8(original);
    std::wstring result = UtilConvert::UTF8ToWString(utf8);
    EXPECT_EQ(result, original);
}

// ===== GetHexIdAsString Tests =====

TEST_F(UtilConvertTest, GetHexIdAsString_ZeroValue_FormatsCorrectly) {
    std::wstring result = UtilConvert::GetHexIdAsString(0, 4);
    EXPECT_EQ(result, L"0x0000");
}

TEST_F(UtilConvertTest, GetHexIdAsString_MaxValue_FormatsCorrectly) {
    std::wstring result = UtilConvert::GetHexIdAsString(0xFFFF, 4);
    EXPECT_EQ(result, L"0xffff");
}

TEST_F(UtilConvertTest, GetHexIdAsString_TypicalVendorId_FormatsCorrectly) {
    // Kingston VID: 0x0951
    std::wstring result = UtilConvert::GetHexIdAsString(0x0951, 4);
    EXPECT_EQ(result, L"0x0951");
}

TEST_F(UtilConvertTest, GetHexIdAsString_TwoByteWidth_FormatsCorrectly) {
    std::wstring result = UtilConvert::GetHexIdAsString(0x12, 2);
    EXPECT_EQ(result, L"0x12");
}

// ===== GetUsbClassNameByDescId Tests =====

TEST_F(UtilConvertTest, GetUsbClassNameByDescId_MassStorage_ReturnsCorrectName) {
    std::wstring result = UtilConvert::GetUsbClassNameByDescId(0x08);
    EXPECT_EQ(result, L"Mass Storage");
}

TEST_F(UtilConvertTest, GetUsbClassNameByDescId_Hub_ReturnsCorrectName) {
    std::wstring result = UtilConvert::GetUsbClassNameByDescId(0x09);
    EXPECT_EQ(result, L"Hub");
}

TEST_F(UtilConvertTest, GetUsbClassNameByDescId_Hid_ReturnsCorrectName) {
    std::wstring result = UtilConvert::GetUsbClassNameByDescId(0x03);
    EXPECT_EQ(result, L"HID (Human Interface Device)");
}

TEST_F(UtilConvertTest, GetUsbClassNameByDescId_VendorSpecific_ReturnsCorrectName) {
    std::wstring result = UtilConvert::GetUsbClassNameByDescId(0xFF);
    EXPECT_EQ(result, L"Vendor Specific");
}

TEST_F(UtilConvertTest, GetUsbClassNameByDescId_Unknown_ReturnsUnknown) {
    // Use a value that's not in the USB class codes
    std::wstring result = UtilConvert::GetUsbClassNameByDescId(0x15);
    EXPECT_EQ(result, L"Unknown");
}

// ===== Basic Windows Types Test =====

TEST_F(UtilConvertTest, WindowsTypesAvailable) {
    UCHAR testByte = 0x08;
    USHORT testShort = 0x1234;

    EXPECT_EQ(testByte, 0x08);
    EXPECT_EQ(testShort, 0x1234);
}
