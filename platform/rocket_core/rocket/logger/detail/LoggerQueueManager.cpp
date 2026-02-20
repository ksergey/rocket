// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "LoggerQueueManager.h"

#include <mutex>

namespace rocket::logger::detail {

auto LoggerQueueManager::createProducer(std::optional<std::size_t> capacityHint) noexcept -> LoggerQueue::Producer {
    if (!capacityHint) {
        capacityHint = this->queueCapacityHint();
    }

    auto [producer, consumer] = LoggerQueue::createProducerAndConsumer("logger-queue", *capacityHint);
    if (!producer || !consumer) {
        return {};
    }

    {
        std::lock_guard guard(pendingAddQueuesLock_);
        pendingAddQueues_.push_back(std::move(consumer));
        rebuildQueuesFlag_.store(true, std::memory_order_relaxed);
    }

    return std::move(producer);
}

void LoggerQueueManager::rebuildQueues() {
    // Drop closed queues
    std::erase_if(queues_, [](LoggerQueue::Consumer const& consumer) {
        assert(static_cast<bool>(consumer));
        return consumer.isClosed();
    });

    // Add pending queues
    std::lock_guard guard(pendingAddQueuesLock_);
    if (!pendingAddQueues_.empty()) {
        for (LoggerQueue::Consumer& consumer : pendingAddQueues_) {
            assert(static_cast<bool>(consumer));
            queues_.push_back(std::move(consumer));
        }
        pendingAddQueues_.clear();
    }
}

} // namespace rocket::logger::detail
