// Custom test main with CRT debug memory tracking enabled
//
// This file replaces gtest_main to enable memory leak detection
// in debug builds. Memory leaks are reported to the Output window
// when tests complete.
//
// NOTE: spdlog allocates internal structures (default logger, thread pool)
// that appear as "leaks" because they are static objects cleaned up after
// CRT's atexit handler. These are expected and not actual memory leaks.
// Typically 5 blocks totaling ~270 bytes from spdlog initialization.

#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

int main(int argc, char** argv)
{
#ifdef _DEBUG
    // Enable CRT memory tracking with automatic leak check at exit
    // _CRTDBG_ALLOC_MEM_DF: Enable debug heap allocations
    // _CRTDBG_LEAK_CHECK_DF: Report leaks at program exit
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Send all reports to stderr (visible in console output)
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    // Uncomment to break on a specific allocation number from leak report
    // _CrtSetBreakAlloc(123);
#endif

    // Suppress verbose spdlog output during tests - only show warnings and errors
    spdlog::set_level(spdlog::level::warn);

    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run all tests
    int result = RUN_ALL_TESTS();

    // Shutdown spdlog to release as many resources as possible
    // Note: Some static allocations may still be reported as leaks
    spdlog::shutdown();

    return result;
}
