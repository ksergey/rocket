// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "Backend.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <csignal>
#include <string_view>
#include <thread>

#include <fmt/format.h>

namespace rocket::logger::detail {
namespace {

struct Signal {
    int sigNo;
    std::string_view sigName;
};

constexpr auto kFailureSignals = std::array{
    Signal{SIGSEGV, "SIGSEGV"},
    Signal{SIGILL, "SIGILL"},
    Signal{SIGFPE, "SIGFPE"},
    Signal{SIGABRT, "SIGABRT"},
    Signal{SIGBUS, "SIGBUS"},
};

[[nodiscard]] auto threadShouldExit() noexcept -> bool {
    static std::atomic<std::size_t> firstExit{0};
    return 0 == firstExit.fetch_add(1, std::memory_order_relaxed);
}

void failureSignalHandler(
    [[maybe_unused]] int sigNo, [[maybe_unused]] siginfo_t* sigInfo, [[maybe_unused]] void* ucontext) {
    auto const found = std::ranges::find_if(kFailureSignals, [&](auto const& sigInfo) {
        return sigInfo.sigNo == sigNo;
    });
    if (found != kFailureSignals.end()) {
        fmt::print(stderr, "Catched signal {}\n", found->sigName);
    } else {
        fmt::print(stderr, "Catched signal no {}\n", sigNo);
    }

    if (!threadShouldExit()) {
        std::this_thread::sleep_for(std::chrono::seconds{30});
    }

    Backend::instance()->stop();
}

void installAtExitHandler() noexcept {
    auto const rc = std::atexit([] {
        Backend::instance()->stop();
    });
    if (rc != 0) {
        fmt::print(stderr, "Can't install at-exit handler\n");
    }
}

void installFailureSignalHandler() noexcept {
    struct ::sigaction sa{};
    ::sigemptyset(&sa.sa_mask);
    sa.sa_flags |= SA_SIGINFO;
    sa.sa_sigaction = &failureSignalHandler;
    for (auto const& sigInfo : kFailureSignals) {
        if (auto const rc = ::sigaction(sigInfo.sigNo, &sa, nullptr); rc == -1) {
            fmt::print(stderr, "Can't install signal handler for {} signal\n", sigInfo.sigName);
        }
    }
}

} // namespace

void Backend::start(std::unique_ptr<Sink> sink, BackendOptions const& options) {
    std::lock_guard guard{backendThreadMutex_};

    std::call_once(shutdownHandlesInstalledFlag_, [] {
        installAtExitHandler();
        installFailureSignalHandler();
    });

    backendThread_.start(std::move(sink), options);
}

void Backend::stop() {
    std::lock_guard guard{backendThreadMutex_};
    backendThread_.stop();
}

} // namespace rocket::logger::detail
