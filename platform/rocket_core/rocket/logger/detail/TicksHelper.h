// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <ctime>
#include <ranges>

#include <fmt/format.h>

#include "../../Platform.h"

namespace rocket::logger::detail {

// See https://github.com/odygrd/quill/blob/master/quill/src/detail/misc/RdtscClock.cpp
class TicksHelper {
  private:
    std::int64_t wallClockBase_{0};
    std::int64_t tscClockBase_{0};
    std::int64_t resyncIntervalTicks_ = 0;
    std::int64_t resyncIntervalOriginal_ = 0;
    double nanosecondsPerTick_{0.0};

  public:
    [[nodiscard]] ROCKET_FORCE_INLINE static auto instance() noexcept -> TicksHelper* {
        static TicksHelper instance;
        return &instance;
    }

    TicksHelper(TicksHelper const&) = delete;
    TicksHelper& operator=(TicksHelper const&) = delete;

    [[nodiscard]] ROCKET_FORCE_INLINE auto timeSinceEpoch(std::int64_t value) noexcept -> std::int64_t {
        auto diff = value - tscClockBase_;
        if (diff > resyncIntervalTicks_) {
            this->sync(std::chrono::nanoseconds(2500));
            diff = value - tscClockBase_;
        }
        return wallClockBase_ + static_cast<std::int64_t>(diff * this->nanosecondsPerTick());
    }

    [[nodiscard]] ROCKET_FORCE_INLINE auto nanosecondsPerTick() const noexcept -> double {
        return nanosecondsPerTick_;
    }

  private:
    TicksHelper(std::chrono::nanoseconds resyncInterval = std::chrono::milliseconds(700)) {
        nanosecondsPerTick_ = calcNanosecondsPerTick();
        resyncIntervalTicks_ = resyncInterval.count() * nanosecondsPerTick_;
        resyncIntervalOriginal_ = resyncIntervalTicks_;
        if (auto const result = sync(std::chrono::nanoseconds(2500)) || sync(std::chrono::microseconds(10)); !result) {
            fmt::print(stderr, "Failed to sync TSC clock\n");
        }
    }

    auto sync(std::chrono::nanoseconds lag) noexcept -> bool {
        constexpr auto kMaxAttempts = std::size_t(4);

        for ([[maybe_unused]] auto attempt : std::views::iota(1) | std::views::take(kMaxAttempts)) {
            auto const tscClockStart = tscClockNow();
            auto const wallClockTime = wallClockNow();
            auto const tscClockStop = tscClockNow();

            if (tscClockStop - tscClockStart <= static_cast<std::int64_t>(lag.count())) [[likely]] {
                wallClockBase_ = wallClockTime;
                tscClockBase_ = fastAvg(tscClockStart, tscClockStop);
                resyncIntervalTicks_ = resyncIntervalOriginal_;
                return true;
            }
        }
        resyncIntervalTicks_ = resyncIntervalTicks_ * 2;
        return false;
    }

    [[nodiscard]] ROCKET_FORCE_INLINE static auto fastAvg(std::int64_t x, std::int64_t y) noexcept -> std::int64_t {
        return (x & y) + ((x ^ y) >> 1);
    }

    // Get current timepoint in nanosecond resolution
    [[nodiscard]] ROCKET_FORCE_INLINE static auto wallClockNow() noexcept -> std::int64_t {
        ::timespec ts;
        ::clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec * 1000000000l + ts.tv_nsec;
    }

    // Get current timepoint in ticks resolution
    [[nodiscard]] ROCKET_FORCE_INLINE static auto tscClockNow() noexcept -> std::int64_t {
        return __builtin_ia32_rdtsc();
    }

    [[nodiscard]] static auto calcNanosecondsPerTick() noexcept -> double {
        constexpr auto kSpinDuration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(10));
        constexpr auto kTrials = std::size_t(13);

        std::array<double, kTrials> rates = {{0}};

        for (double& rate : rates) {
            auto const wallClockStart = wallClockNow();
            auto const tscClockStart = tscClockNow();

            std::int64_t wallClockElapsed;
            std::int64_t tscClockStop;
            do {
                auto const wallClockStop = wallClockNow();
                tscClockStop = tscClockNow();
                wallClockElapsed = wallClockStop - wallClockStart;
            } while (wallClockElapsed < kSpinDuration.count());

            rate = static_cast<double>(tscClockStop - tscClockStart) / static_cast<double>(wallClockElapsed);
        }

        std::nth_element(rates.begin(), rates.begin() + kTrials / 2, rates.end());

        return 1.0 / rates[kTrials / 2];
    }
};

} // namespace rocket::logger::detail
