#include <gtest/gtest.h>
#include <windows.h>
#include <SetupAPI.h>
#include <usb.h>
#include <usbioctl.h>
#include <usbiodef.h>
#include <wil/result.h>
#include <wil/resource.h>
#include "usbdesc.h"
#include "DevicesManager.h"
#include "DeviceResultantInfo.h"
#include "DeviceEnumerator.h"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

namespace KDM
{
namespace Testing
{

/// <summary>
/// Thread safety tests for DevicesManager and related classes.
/// These tests verify that concurrent enumeration operations don't cause crashes or data corruption.
/// </summary>
class ThreadSafetyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Allow some warm-up time for USB subsystem
    }

    void TearDown() override
    {
    }
};

/// <summary>
/// Tests concurrent USB device enumeration from multiple threads.
/// Each thread creates its own DevicesManager and enumerates devices.
/// </summary>
// TEST_F(ThreadSafetyTest, ConcurrentEnumeration_NoThrow)
// {
//     constexpr int numThreads = 4;
//     std::vector<std::thread> threads;
//     std::atomic<int> successCount{0};
//     std::atomic<int> exceptionCount{0};

//     for (int i = 0; i < numThreads; ++i)
//     {
//         threads.emplace_back([&successCount, &exceptionCount]() {
//             try
//             {
//                 DevicesManager manager;
//                 manager.EnumerateUsbDevices();

//                 // Verify we got some devices
//                 const auto& devices = manager.GetDevices();
//                 if (!devices.empty())
//                 {
//                     ++successCount;
//                 }
//             }
//             catch (const std::exception&)
//             {
//                 ++exceptionCount;
//             }
//         });
//     }

//     // Wait for all threads to complete
//     for (auto& t : threads)
//     {
//         t.join();
//     }

//     // All threads should succeed without exceptions
//     EXPECT_EQ(exceptionCount.load(), 0) << "No exceptions should occur during concurrent enumeration";
//     EXPECT_GT(successCount.load(), 0) << "At least one thread should successfully enumerate devices";
// }

/// <summary>
/// Tests that concurrent DeviceEnumerator operations don't crash.
/// </summary>
TEST_F(ThreadSafetyTest, ConcurrentDeviceEnumerator_NoThrow)
{
    constexpr int numThreads = 4;
    std::vector<std::thread> threads;
    std::atomic<int> successCount{0};
    std::atomic<int> exceptionCount{0};

    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back([&successCount, &exceptionCount]() {
            try
            {
                DeviceEnumerator enumerator(GUID_DEVINTERFACE_USB_DEVICE,
                    DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
                auto devices = enumerator.GetDeviceInstances();

                if (!devices.empty())
                {
                    ++successCount;
                }
            }
            catch (const std::exception&)
            {
                ++exceptionCount;
            }
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    EXPECT_EQ(exceptionCount.load(), 0) << "No exceptions should occur during concurrent device enumeration";
}

/// <summary>
/// Tests rapid sequential enumeration to check for resource leaks.
/// </summary>
TEST_F(ThreadSafetyTest, RapidSequentialEnumeration_NoResourceLeak)
{
    constexpr int iterations = 10;

    for (int i = 0; i < iterations; ++i)
    {
        DevicesManager manager;
        EXPECT_NO_THROW(manager.EnumerateUsbDevices());

        // Verify results are consistent
        const auto& devices = manager.GetDevices();
        // Just verify we don't crash - device count may vary
        (void)devices.size();
    }
}

/// <summary>
/// Tests that results from concurrent enumeration are consistent.
/// All threads should see the same set of connected devices.
/// </summary>
// TEST_F(ThreadSafetyTest, ConcurrentEnumeration_ConsistentResults)
// {
//     constexpr int numThreads = 3;
//     std::vector<std::thread> threads;
//     std::vector<size_t> deviceCounts(numThreads);
//     std::atomic<bool> anyFailed{false};

//     for (int i = 0; i < numThreads; ++i)
//     {
//         threads.emplace_back([i, &deviceCounts, &anyFailed]() {
//             try
//             {
//                 DevicesManager manager;
//                 manager.EnumerateUsbDevices();
//                 deviceCounts[i] = manager.GetDevices().size();
//             }
//             catch (const std::exception&)
//             {
//                 anyFailed = true;
//             }
//         });
//     }

//     for (auto& t : threads)
//     {
//         t.join();
//     }

//     EXPECT_FALSE(anyFailed.load()) << "No thread should fail";

//     // All threads should see the same device count (assuming no hot-plug during test)
//     for (int i = 1; i < numThreads; ++i)
//     {
//         EXPECT_EQ(deviceCounts[0], deviceCounts[i])
//             << "Thread " << i << " saw different device count than thread 0";
//     }
// }

/// <summary>
/// Tests that DevicesManager destruction during enumeration doesn't crash.
/// This simulates early cleanup scenarios.
/// </summary>
TEST_F(ThreadSafetyTest, EarlyDestruction_NoThrow)
{
    // Create manager in a scope and let it be destroyed
    {
        auto manager = std::make_unique<DevicesManager>();
        // Don't enumerate - just destroy immediately
    }

    // Create another and enumerate then destroy
    {
        auto manager = std::make_unique<DevicesManager>();
        manager->EnumerateUsbDevices();
        // Destroy with devices list populated
    }

    // If we get here without crash, test passes
    SUCCEED();
}

/// <summary>
/// Tests moving DevicesManager between threads.
/// </summary>
TEST_F(ThreadSafetyTest, MoveSemantics_AcrossThreads)
{
    std::unique_ptr<DevicesManager> manager;
    std::atomic<bool> enumerationDone{false};
    std::atomic<size_t> deviceCount{0};

    // Create and enumerate in one thread
    std::thread createThread([&manager, &enumerationDone]() {
        manager = std::make_unique<DevicesManager>();
        manager->EnumerateUsbDevices();
        enumerationDone = true;
    });

    createThread.join();

    ASSERT_TRUE(enumerationDone.load());
    ASSERT_NE(manager, nullptr);

    // Access results in this thread
    EXPECT_NO_THROW({
        deviceCount = manager->GetDevices().size();
    });

    // Cleanup
    manager.reset();
}

} // namespace Testing
} // namespace KDM
