// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include <fmt/format.h>

#include "../BackendOptions.h"
#include "../Sink.h"
#include "LoggerQueueManager.h"

namespace rocket::logger::detail {

class BackendThread final {
  private:
    LoggerQueueManager& queueManager_;
    // Backend thread
    std::jthread thread_;
    // Flag indicates backend thread running
    std::atomic<bool> running_{false};
    // Cache for message formatting
    fmt::memory_buffer formatBuffer_;

  public:
    BackendThread(BackendThread const&) = delete;
    BackendThread& operator=(BackendThread const&) = delete;

    /// Constructor
    explicit BackendThread(LoggerQueueManager& queueManager);

    /// Destructor
    ~BackendThread();

    /// Return true on backend thread running
    [[nodiscard]] ROCKET_FORCE_INLINE auto isRunning() const noexcept {
        return running_.load(std::memory_order_acquire);
    }

    /// Start backend thread
    void start(std::unique_ptr<Sink> sink, BackendOptions const& options);

    /// Stop backend thread
    void stop();

  private:
    auto processIncomingLogRecords(Sink& sink) -> std::size_t;
    void processLogRecord(Sink& sink, LogRecordHeader const* logRecordHeader, RecordMetadata const* metadata,
        std::byte const* argsBuffer);
};

} // namespace rocket::logger::detail
