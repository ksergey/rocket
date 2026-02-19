// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <atomic>
#include <concepts>
#include <csignal>
#include <functional>

#include "Platform.h"

namespace rocket {

extern std::atomic<std::uint64_t> gPendingSignals;

namespace detail {

template <int SigNo>
[[nodiscard]] constexpr auto isSignalBitSet(std::uint64_t value) noexcept -> bool {
    return ((1ull << SigNo) & value) == (1ull << SigNo);
}

} // namespace detail

/// Install signal handlers
void installSignalHandlers();

/// Restore default signal handlers
void restoreSignalHandlers();

/// Helper for manage active signals
struct CatchedSignal {
    std::uint64_t signals;

    /// Return true on shutdown signals catched
    [[nodiscard]] constexpr auto shutdown() const noexcept -> bool {
        return detail::isSignalBitSet<SIGINT>(signals) || detail::isSignalBitSet<SIGTERM>(signals);
    }

    /// Return true on reload signals catched
    [[nodiscard]] constexpr auto reload() const noexcept -> bool {
        return detail::isSignalBitSet<SIGHUP>(signals);
    }
};

/// Notify on a signal catched
template <typename Fn>
    requires std::invocable<Fn, CatchedSignal>
ROCKET_FORCE_INLINE void notifyCatchedSignals(Fn&& fn) {
    auto const pendingSignals = gPendingSignals.exchange(0, std::memory_order_relaxed);
    if (pendingSignals > 0) [[unlikely]] {
        std::invoke(std::forward<Fn>(fn), CatchedSignal{pendingSignals});
    }
}

} // namespace rocket
