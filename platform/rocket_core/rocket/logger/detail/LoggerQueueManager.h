// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <atomic>
#include <cassert>
#include <optional>
#include <ranges>
#include <vector>

#include "../../SpinLock.h"
#include "LoggerQueue.h"

namespace rocket::logger::detail {

/// Default queue capacity hint
constexpr std::size_t kDefaultCapacityHint = 2 * 1024 * 1024;

/// Queue manager
/// Used for queues lifetime
class LoggerQueueManager final {
  private:
    std::vector<LoggerQueue::Consumer> queues_;
    std::vector<LoggerQueue::Consumer> pendingAddQueues_;
    SpinLock pendingAddQueuesLock_;
    std::atomic<bool> rebuildQueuesFlag_{false};
    // Capacity for a new queues
    std::atomic<std::size_t> queueCapacityHint_{kDefaultCapacityHint};

  public:
    LoggerQueueManager(LoggerQueueManager const&) = delete;
    LoggerQueueManager& operator=(LoggerQueueManager const&) = delete;

    LoggerQueueManager() = default;

    [[nodiscard]] auto queueCapacityHint() const noexcept -> std::size_t {
        return queueCapacityHint_.load(std::memory_order_relaxed);
    }

    /// Set a queue capacity hint (for new queues)
    /// @throw std::runtime_error on value is zero
    void setQueueCapacityHint(std::size_t value) {
        if (value == 0) [[unlikely]] {
            throw std::runtime_error("queue capacity hint is out of range");
        }
        queueCapacityHint_.store(value, std::memory_order_relaxed);
    }

    /// Create producer with default (or requested) capacity hint
    [[nodiscard]] auto createProducer(std::optional<std::size_t> capacityHint = {}) noexcept -> LoggerQueue::Producer;

    /// Iterate over active queues
    /// Non-thread safe
    template <typename Fn>
        requires std::invocable<Fn, LoggerQueue::Consumer*>
    void forEachConsumer(Fn&& fn) {
        if (rebuildQueuesFlag_.load(std::memory_order_relaxed)) [[unlikely]] {
            this->rebuildQueues();
            rebuildQueuesFlag_.store(false, std::memory_order_relaxed);
        }

        bool atLeastOneQueueClosed{false};
        for (auto& consumer : queues_) {
            assert(static_cast<bool>(consumer));

            std::invoke(std::forward<Fn>(fn), &consumer);
            atLeastOneQueueClosed = atLeastOneQueueClosed || consumer.isClosed();
        }

        if (atLeastOneQueueClosed) {
            rebuildQueuesFlag_.store(true, std::memory_order_relaxed);
        }
    }

  private:
    void rebuildQueues();
};

} // namespace rocket::logger::detail
