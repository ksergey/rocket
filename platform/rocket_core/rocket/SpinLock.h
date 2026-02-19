// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <immintrin.h>

#include <atomic>

namespace rocket {

/// mimic: std::mutex
/// std::lock_guard works!
class SpinLock final {
  private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

  public:
    /// Locks the SpinLock, blocks if SpinLock is not available
    void lock() noexcept {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            _mm_pause();
        }
    }

    /// Tries to lock the SpinLock, returns if the SpinLock is not available
    [[nodiscard]] auto try_lock() noexcept -> bool {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    /// Unlocks the SpinLock
    void unlock() noexcept {
        flag_.clear(std::memory_order_release);
    }
};

} // namespace rocket
