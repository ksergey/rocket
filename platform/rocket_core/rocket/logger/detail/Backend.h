// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <atomic>
#include <memory>
#include <mutex>

#include "../../Platform.h"
#include "../BackendOptions.h"
#include "../Common.h"
#include "../Sink.h"
#include "BackendThread.h"
#include "ThreadContext.h"

namespace rocket::logger::detail {

class Backend {
  private:
    alignas(kHardwareDestructiveInterferenceSize) std::atomic<LogLevel> logLevel_{LogLevel::Notice};
    std::once_flag shutdownHandlesInstalledFlag_;
    LoggerQueueManager loggerQueueManager_;
    BackendThread backendThread_{loggerQueueManager_};
    std::mutex backendThreadMutex_;

  public:
    [[nodiscard]] ROCKET_FORCE_INLINE static auto instance() -> Backend* {
        static Backend instance;
        return &instance;
    }

    Backend(Backend const&) = delete;
    Backend& operator=(Backend const&) = delete;

    /// Current log verbosity level
    [[nodiscard]] ROCKET_FORCE_INLINE auto logLevel() const noexcept {
        return logLevel_.load(std::memory_order_relaxed);
    }

    /// Change log verbosity level
    ROCKET_FORCE_INLINE void setLogLevel(LogLevel value) noexcept {
        logLevel_.store(value, std::memory_order_relaxed);
    }

    /// Return true on message with log verbosity value @c value should be logged
    [[nodiscard]] ROCKET_FORCE_INLINE auto shouldLog(LogLevel value) const noexcept {
        return value <= this->logLevel();
    }

    /// Queue capacity hint
    [[nodiscard]] ROCKET_FORCE_INLINE auto queueCapacityHint() const noexcept -> std::size_t {
        return loggerQueueManager_.queueCapacityHint();
    }

    /// Change queue capacity hint
    void setQueueCapacityHint(std::size_t value) {
        loggerQueueManager_.setQueueCapacityHint(value);
    }

    /// Get ThreadContext for current thread
    [[nodiscard]] ROCKET_FORCE_INLINE auto localThreadContext() noexcept -> ThreadContext* {
        static thread_local auto threadContext = ThreadContext{loggerQueueManager_};
        return &threadContext;
    }

    /// Return true on backend ready to process log records
    [[nodiscard]] ROCKET_FORCE_INLINE auto isReady() const noexcept {
        return backendThread_.isRunning();
    }

    /// Start backend thread
    void start(std::unique_ptr<Sink> sink, BackendOptions const& options);

    /// Stop backend thread
    void stop();

  private:
    Backend() = default;
};

} // namespace rocket::logger::detail
