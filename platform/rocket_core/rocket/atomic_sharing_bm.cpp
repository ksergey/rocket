// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <atomic>
#include <chrono>
#include <cstdint>
#include <print>
#include <string_view>
#include <thread>

#include "Platform.h"

using rocket::kHardwareConstructiveInterferenceSize;
using rocket::kHardwareDestructiveInterferenceSize;

constexpr auto kMaxWriteIterations = std::size_t{10'000'000}; // the benchmark time tuning
constexpr auto kMaxRuns = std::size_t{8};

struct alignas(kHardwareConstructiveInterferenceSize) OneCacheLiner // occupies one cache line
{
    std::atomic_uint64_t x{};
    std::atomic_uint64_t y{};
};

struct TwoCacheLiner // occupies two cache lines
{
    alignas(kHardwareDestructiveInterferenceSize) std::atomic_uint64_t x{};
    alignas(kHardwareDestructiveInterferenceSize) std::atomic_uint64_t y{};
};

ROCKET_NO_INLINE auto now() noexcept {
    return std::chrono::high_resolution_clock::now();
}

template <bool xy, typename CacheLinerT>
void cacheLinerThread(CacheLinerT& cacheLiner, std::size_t runNo) {
    auto const start = now();
    for (std::size_t count = 0; count != kMaxWriteIterations; ++count) {
        if constexpr (xy) {
            cacheLiner.x.fetch_add(1, std::memory_order_relaxed);
        } else {
            cacheLiner.y.fetch_add(1, std::memory_order_relaxed);
        }
    }
    std::chrono::duration<double, std::milli> const elapsed = now() - start;
    std::print("  {}.{}: spent {} ms\n", runNo, int(xy), elapsed.count());

    if constexpr (xy) {
        cacheLiner.x = elapsed.count();
    } else {
        cacheLiner.y = elapsed.count();
    }
}

template <typename CacheLinerT>
void run(std::string_view name) {
    std::print("Run --- {} ---\n", name);
    std::print(" size = {}\n", sizeof(CacheLinerT));

    auto cacheLiner = CacheLinerT{};
    auto avg = int{0};
    for (auto i = std::size_t{0}; i != kMaxRuns; ++i) {
        auto th1 = std::thread([&] {
            cacheLinerThread<0>(cacheLiner, i);
        });
        auto th2 = std::thread([&] {
            cacheLinerThread<1>(cacheLiner, i);
        });
        th1.join();
        th2.join();
        avg += cacheLiner.x + cacheLiner.y;
    }
    std::print("Average time: {} ms\n\n", (avg / kMaxRuns / 2));
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> int {
    std::print("Based on\n");
    std::print("  https://en.cppreference.com/w/cpp/thread/hardware_destructive_interference_size.html\n");
    std::print("\n");
    std::print("kHardwareDestructiveInterferenceSize = {}\n", kHardwareDestructiveInterferenceSize);
    std::print("kHardwareConstructiveInterferenceSize = {}\n", kHardwareConstructiveInterferenceSize);
    std::print("\n");

    run<OneCacheLiner>("OneCacheLiner");
    run<TwoCacheLiner>("TwoCacheLiner");

    return 0;
}
