cmake_minimum_required(VERSION 3.20)

# Unit tests for WinDevices core library
project(WinDevicesTests)

# Test source files
set(TEST_SOURCES
    TestMain.cpp
    DeviceEnumeratorTests.cpp
    DevicesManagerTests.cpp
    DeviceCommunicationTests.cpp
    ThreadSafetyTests.cpp
    UtilConvertTests.cpp
    UsbHubMockTests.cpp
    PropertyBasedTests.cpp
)

# Create test executable
add_executable(WinDevicesTests ${TEST_SOURCES})

# Link libraries
# Note: Using GTest::gtest instead of GTest::gtest_main because we have
# our own TestMain.cpp with CRT debug memory tracking enabled
target_link_libraries(WinDevicesTests
    PRIVATE
        WinDevicesCore
        spdlog::spdlog
        GTest::gtest
        GTest::gmock
)

# Include directories
target_include_directories(WinDevicesTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/WinDevices
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

# Discover tests for CTest
gtest_discover_tests(WinDevicesTests)

# Set test properties
set_target_properties(WinDevicesTests PROPERTIES
    FOLDER "Tests"
)
