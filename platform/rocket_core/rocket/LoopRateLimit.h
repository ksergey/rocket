// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <thread>

#include "Platform.h"

namespace rocket {

/// Limit process loop min iteration time
class LoopRateLimit {
  private:
    std::chrono::nanoseconds minLoopDuration_;
    std::int64_t deadline_;

  public:
    explicit LoopRateLimit(std::chrono::nanoseconds minLoopDuration = std::chrono::milliseconds(100))
        : minLoopDuration_{minLoopDuration}, deadline_{wallClockNow() + minLoopDuration_.count()} {}

    ROCKET_FORCE_INLINE void sleep() noexcept {
        auto const now = wallClockNow();
        if (now < deadline_) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(deadline_ - now));
            deadline_ += minLoopDuration_.count();
        } else {
            deadline_ = now + minLoopDuration_.count();
        }
    }

    ROCKET_FORCE_INLINE void reset() noexcept {
        deadline_ = 0;
    }

  private:
    // Get current timepoint in nanosecond resolution
    [[nodiscard]] ROCKET_FORCE_INLINE static auto wallClockNow() noexcept -> std::int64_t {
        ::timespec ts;
        ::clock_gettime(CLOCK_MONOTONIC, &ts);
        return ts.tv_sec * 1000000000l + ts.tv_nsec;
    }
};

} // namespace rocket
