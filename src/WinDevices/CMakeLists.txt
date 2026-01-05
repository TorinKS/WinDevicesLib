cmake_minimum_required(VERSION 3.20)

# =============================================================================
# WinDevices Core Library (Static)
# =============================================================================

# Collect source files
set(WINDEVICES_SOURCES
    UsbDeviceClassInfo.cpp
    DeviceCommunication.cpp
    DeviceEnumerator.cpp
    DeviceInfo.cpp
    DeviceProperty.cpp
    DeviceResultantInfo.cpp
    DevicesManager.cpp
    DevInfoData.cpp
    HubConnectionInfo.cpp
    HubNodeCapabilities.cpp
    HubNodeInfo.cpp
    HubNodeInfoEx.cpp
    HubPortInfo.cpp
    pch.cpp
    UsbDeviceDescriptorInfo.cpp
    UsbDescriptorParser.cpp
    UsbHostController.cpp
    UsbHub.cpp
    UsbPortInfo.cpp
    UsbVendorList.cpp
    UtilConvert.cpp
)

# Collect header files (located in include/WinDevices)
set(WINDEVICES_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/WinDevices)
set(WINDEVICES_HEADERS
    ${WINDEVICES_INCLUDE_DIR}/UsbDeviceClassInfo.h
    ${WINDEVICES_INCLUDE_DIR}/DeviceCommunication.h
    ${WINDEVICES_INCLUDE_DIR}/DeviceEnumerator.h
    ${WINDEVICES_INCLUDE_DIR}/DeviceInfo.h
    ${WINDEVICES_INCLUDE_DIR}/DeviceProperty.h
    ${WINDEVICES_INCLUDE_DIR}/DeviceResultantInfo.h
    ${WINDEVICES_INCLUDE_DIR}/DevicesManager.h
    ${WINDEVICES_INCLUDE_DIR}/DevInfoData.h
    ${WINDEVICES_INCLUDE_DIR}/framework.h
    ${WINDEVICES_INCLUDE_DIR}/HubConnectionInfo.h
    ${WINDEVICES_INCLUDE_DIR}/HubNodeCapabilitiesEx.h
    ${WINDEVICES_INCLUDE_DIR}/HubNodeInfo.h
    ${WINDEVICES_INCLUDE_DIR}/HubNodeInfoEx.h
    ${WINDEVICES_INCLUDE_DIR}/HubPortInfo.h
    ${WINDEVICES_INCLUDE_DIR}/IDeviceCommunication.h
    ${WINDEVICES_INCLUDE_DIR}/IDeviceEnumerator.h
    ${WINDEVICES_INCLUDE_DIR}/pch.h
    ${WINDEVICES_INCLUDE_DIR}/usbdesc.h
    ${WINDEVICES_INCLUDE_DIR}/UsbDescriptorParser.h
    ${WINDEVICES_INCLUDE_DIR}/UsbDeviceDescriptorInfo.h
    ${WINDEVICES_INCLUDE_DIR}/UsbHostController.h
    ${WINDEVICES_INCLUDE_DIR}/UsbHub.h
    ${WINDEVICES_INCLUDE_DIR}/UsbPortInfo.h
    ${WINDEVICES_INCLUDE_DIR}/UsbVendorList.h
    ${WINDEVICES_INCLUDE_DIR}/UtilConvert.h
    ${WINDEVICES_INCLUDE_DIR}/CrtDebug.h
    ${WINDEVICES_INCLUDE_DIR}/DeviceClassGuids.h
    ${WINDEVICES_INCLUDE_DIR}/Exceptions.h
    ${WINDEVICES_INCLUDE_DIR}/UsbClassCodes.h
)

# Create library target
add_library(WinDevicesCore ${WINDEVICES_SOURCES} ${WINDEVICES_HEADERS})

# Create alias for consistent naming (supports both find_package and add_subdirectory usage)
add_library(WinDevices::Core ALIAS WinDevicesCore)

# Set target properties
set_target_properties(WinDevicesCore PROPERTIES
    OUTPUT_NAME "WinDevicesCore"
    POSITION_INDEPENDENT_CODE ON
    EXPORT_NAME Core
)

# Include directories with proper generator expressions
target_include_directories(WinDevicesCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include/WinDevices>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries (all PRIVATE since public API doesn't expose these dependencies)
target_link_libraries(WinDevicesCore
    PRIVATE
        spdlog::spdlog
        WIL::WIL
        setupapi.lib
        cfgmgr32.lib
)

# Precompiled headers
target_precompile_headers(WinDevicesCore PRIVATE ${WINDEVICES_INCLUDE_DIR}/pch.h)

# Install rules with component (no export - internal static library)
install(TARGETS WinDevicesCore
    COMPONENT WinDevices
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${WINDEVICES_HEADERS}
    DESTINATION include/WinDevices
    COMPONENT WinDevices
)

# =============================================================================
# WinDevices C API Library (Shared/DLL) for .NET P/Invoke interop
# =============================================================================

set(C_API_SOURCES
    WinDevicesAPI.cpp
)

set(C_API_HEADERS
    WinDevicesAPI.h
)

# Create shared library for C API
add_library(WinDevicesAPI SHARED ${C_API_SOURCES} ${C_API_HEADERS})

# Create alias for consistent naming
add_library(WinDevices::API ALIAS WinDevicesAPI)

# Set target properties
set_target_properties(WinDevicesAPI PROPERTIES
    OUTPUT_NAME "WinDevices"
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    EXPORT_NAME API
)

# Include directories
target_include_directories(WinDevicesAPI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include/WinDevices
        ${CMAKE_SOURCE_DIR}/include
)

# Link to core library
target_link_libraries(WinDevicesAPI
    PRIVATE
        WinDevicesCore
        spdlog::spdlog
)

# Export symbols for DLL
if(WIN32)
    target_compile_definitions(WinDevicesAPI PRIVATE WINDEVICES_API_EXPORTS)
endif()

# Install rules with component
install(TARGETS WinDevicesAPI
    EXPORT WinDevicesTargets
    COMPONENT WinDevices
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${C_API_HEADERS}
    DESTINATION include
    COMPONENT WinDevices
)
