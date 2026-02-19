// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "Signals.h"

#include <fmt/format.h>

namespace rocket {

std::atomic<std::uint64_t> gPendingSignals{0};

namespace {

ROCKET_COLD void signalHandler(int sigNo) noexcept {
    if (static_cast<std::uint64_t>(sigNo) < sizeof(std::uint64_t) * 8) {
        gPendingSignals.fetch_or((1ull << sigNo), std::memory_order_relaxed);
    } else {
        fmt::print(stderr, "Signal {} catched but ignored\n", sigNo);
    }
}

void installHandler(int sigNo, void (*handler)(int)) noexcept {
    struct ::sigaction sa{};
    ::sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    if (auto const rc = ::sigaction(sigNo, &sa, nullptr); rc == -1) {
        fmt::print(stderr, "Failed to install handler for signal {}\n", sigNo);
    }
}

} // namespace

void installSignalHandlers() {
    for (auto const sigNo : {SIGINT, SIGTERM, SIGHUP}) {
        installHandler(sigNo, signalHandler);
    }
}

void restoreSignalHandlers() {
    for (auto const sigNo : {SIGINT, SIGTERM, SIGHUP}) {
        installHandler(sigNo, SIG_DFL);
    }
}

} // namespace rocket
