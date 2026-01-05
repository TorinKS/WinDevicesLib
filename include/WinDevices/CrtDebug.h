#pragma once

// ============================================================================
// CRT Debug Memory Tracking Utilities
// ============================================================================
// This header provides utilities for enabling CRT debug features in debug builds.
//
// Features:
// - Memory leak detection with file/line information
// - Heap corruption detection
// - Invalid memory access detection
// - Automatic leak report at program exit
//
// Usage:
// 1. Include this header in your main entry point file
// 2. Call CrtDebug::EnableMemoryTracking() at the start of main() or WinMain()
// 3. Memory leaks will be reported to the Output window on program exit
//
// To break on a specific allocation number (shown in leak report):
//   _CrtSetBreakAlloc(allocation_number);
// ============================================================================

#ifdef _DEBUG
#include <crtdbg.h>
#endif

namespace KDM
{

/// <summary>
/// CRT Debug utilities for memory tracking in debug builds.
/// All methods are no-ops in release builds.
/// </summary>
class CrtDebug
{
public:
    /// <summary>
    /// Enables CRT memory tracking features for debug builds.
    /// Call this at the very start of your program (before any allocations).
    ///
    /// Flags enabled:
    /// - _CRTDBG_ALLOC_MEM_DF: Enable debug heap allocations
    /// - _CRTDBG_LEAK_CHECK_DF: Automatic leak report at program exit
    /// </summary>
    static void EnableMemoryTracking() noexcept
    {
#ifdef _DEBUG
        // Get current debug flags
        int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

        // Enable debug heap allocations
        flags |= _CRTDBG_ALLOC_MEM_DF;

        // Enable automatic leak check at program exit
        flags |= _CRTDBG_LEAK_CHECK_DF;

        // Set the new flags
        _CrtSetDbgFlag(flags);
#endif
    }

    /// <summary>
    /// Enables strict memory checking (slower but catches more issues).
    /// Checks heap integrity on every allocation and deallocation.
    /// Use this when debugging memory corruption issues.
    /// </summary>
    static void EnableStrictChecking() noexcept
    {
#ifdef _DEBUG
        int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

        // Check heap integrity on every alloc/free (SLOW but thorough)
        flags |= _CRTDBG_CHECK_ALWAYS_DF;

        // Delay freeing memory to catch use-after-free bugs
        flags |= _CRTDBG_DELAY_FREE_MEM_DF;

        _CrtSetDbgFlag(flags);
#endif
    }

    /// <summary>
    /// Sets a breakpoint on a specific memory allocation number.
    /// The allocation number is shown in the memory leak report.
    /// </summary>
    /// <param name="allocationNumber">Allocation number to break on</param>
    static void BreakOnAllocation(long allocationNumber) noexcept
    {
#ifdef _DEBUG
        _CrtSetBreakAlloc(allocationNumber);
#else
        (void)allocationNumber; // Suppress unused parameter warning
#endif
    }

    /// <summary>
    /// Manually triggers a memory leak check.
    /// Returns true if memory leaks were detected.
    /// </summary>
    /// <returns>true if leaks detected, false otherwise</returns>
    [[nodiscard]] static bool CheckForLeaks() noexcept
    {
#ifdef _DEBUG
        return _CrtDumpMemoryLeaks() != 0;
#else
        return false;
#endif
    }

    /// <summary>
    /// Dumps the current state of all allocated memory blocks to the debug output.
    /// Useful for tracking memory usage at specific points in the program.
    /// </summary>
    static void DumpMemoryState() noexcept
    {
#ifdef _DEBUG
        _CrtMemState state;
        _CrtMemCheckpoint(&state);
        _CrtMemDumpStatistics(&state);
#endif
    }

    /// <summary>
    /// Validates the heap integrity.
    /// Use this to check for heap corruption.
    /// </summary>
    /// <returns>true if heap is valid, false if corrupted</returns>
    [[nodiscard]] static bool ValidateHeap() noexcept
    {
#ifdef _DEBUG
        return _CrtCheckMemory() != 0;
#else
        return true;
#endif
    }

    // Prevent instantiation - this is a static utility class
    CrtDebug() = delete;
    ~CrtDebug() = delete;
    CrtDebug(const CrtDebug&) = delete;
    CrtDebug& operator=(const CrtDebug&) = delete;
};

/// <summary>
/// RAII class that enables memory tracking on construction.
/// Place an instance at the start of main() or in a global initializer.
///
/// Example:
///   int main() {
///       KDM::CrtDebugInitializer debugInit;  // Enables tracking
///       // ... rest of program
///   }  // Leak report generated here
/// </summary>
class CrtDebugInitializer
{
public:
    CrtDebugInitializer() noexcept
    {
        CrtDebug::EnableMemoryTracking();
    }

    /// <summary>
    /// Constructor with option for strict checking.
    /// </summary>
    /// <param name="enableStrictChecking">If true, enables heap integrity checks on every allocation</param>
    explicit CrtDebugInitializer(bool enableStrictChecking) noexcept
    {
        CrtDebug::EnableMemoryTracking();
        if (enableStrictChecking)
        {
            CrtDebug::EnableStrictChecking();
        }
    }

    ~CrtDebugInitializer() = default;

    // Non-copyable, non-movable
    CrtDebugInitializer(const CrtDebugInitializer&) = delete;
    CrtDebugInitializer& operator=(const CrtDebugInitializer&) = delete;
    CrtDebugInitializer(CrtDebugInitializer&&) = delete;
    CrtDebugInitializer& operator=(CrtDebugInitializer&&) = delete;
};

} // namespace KDM
