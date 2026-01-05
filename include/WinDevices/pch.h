// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#pragma once

// ============================================================================
// CRT Debug Memory Tracking (Debug builds only)
// ============================================================================
// Enable memory leak detection and other CRT debug features.
// Must be defined BEFORE including any CRT headers.
//
// Features enabled:
// - _CRTDBG_MAP_ALLOC: Maps malloc/free to debug versions with file/line info
// - _CRTDBG_ALLOC_MEM_DF: Enable debug heap allocations
// - _CRTDBG_LEAK_CHECK_DF: Automatic leak check at program exit
// - _CRTDBG_CHECK_ALWAYS_DF: Check heap integrity on every alloc/free
//
// Usage: Memory leaks are reported to Output window on program exit.
// To break on a specific allocation, set _crtBreakAlloc = <allocation_number>
// ============================================================================
#ifdef _DEBUG
    #define _CRTDBG_MAP_ALLOC
#endif

#include <stdio.h>
#include <stdlib.h>
#include <crtdbg.h>

// Note: We intentionally do NOT define DEBUG_NEW or redefine 'new' globally
// because it conflicts with third-party libraries (WIL, etc.) that use
// placement new or other new-related syntax. The _CRTDBG_MAP_ALLOC macro
// still provides memory leak detection for malloc/free calls.

#include <assert.h>
#include <Windows.h>
#include <SetupAPI.h>
#include <initguid.h>
#include <Usbiodef.h>
#include <GuidDef.h>
#include <usb.h>
#include <usbuser.h>
#include <usbioctl.h>
#include <usbspec.h>
#include "usbdesc.h"

// STL headers
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// WIL headers (MUST come AFTER Windows headers)
#include <wil/result.h>
#include <wil/result_macros.h>
#include <wil/resource.h>
#include <wil/win32_helpers.h>
#include <wil/filesystem.h>
#include <wil/com.h>

#include "Exceptions.h"
#include "CrtDebug.h"

#endif //PCH_H
