cmake_minimum_required(VERSION 3.20)

# End-to-end tests for WinDevices with real USB devices
project(WinDevicesE2ETests)

# E2E test source files
set(E2E_TEST_SOURCES
    UsbMassStorageE2ETests.cpp
    DeviceEnumerationE2ETests.cpp
    DeviceClassEnumerationE2ETests.cpp
)

# Create E2E test executable
add_executable(WinDevicesE2ETests ${E2E_TEST_SOURCES})

# Link libraries
target_link_libraries(WinDevicesE2ETests
    PRIVATE
        WinDevicesAPI
        WinDevicesCore
        GTest::gtest_main
)

# Include directories
target_include_directories(WinDevicesE2ETests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/WinDevices
        ${CMAKE_SOURCE_DIR}/include/WinDevices
        ${CMAKE_SOURCE_DIR}/include
)

# Discover tests for CTest
gtest_discover_tests(WinDevicesE2ETests)

# Set test properties
set_target_properties(WinDevicesE2ETests PROPERTIES
    FOLDER "Tests"
)

# Add custom target to run E2E tests with clear instructions
add_custom_target(run-e2e-tests
    COMMAND ${CMAKE_COMMAND} -E echo "========================================" 
    COMMAND ${CMAKE_COMMAND} -E echo "Running E2E Tests with USB Devices"
    COMMAND ${CMAKE_COMMAND} -E echo "========================================" 
    COMMAND ${CMAKE_COMMAND} -E echo "Please ensure USB mass storage devices are connected!"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND $<TARGET_FILE:WinDevicesE2ETests>
    DEPENDS WinDevicesE2ETests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running end-to-end tests with real USB devices..."
)
