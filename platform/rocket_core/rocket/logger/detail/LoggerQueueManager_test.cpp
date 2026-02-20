// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <doctest/doctest.h>

#include "LoggerQueueManager.h"

namespace rocket::logger::detail {

auto getConsumersCount(LoggerQueueManager& qm) noexcept -> std::size_t {
    std::size_t count = 0;
    qm.forEachConsumer([&]([[maybe_unused]] auto const consumer) {
        count += 1;
    });
    return count;
}

TEST_CASE("LoggerQueueManager") {
    LoggerQueueManager queueManager;

    REQUIRE_EQ(getConsumersCount(queueManager), 0);

    {
        [[maybe_unused]] auto producer = queueManager.createProducer(1024 * 1024);
        REQUIRE_EQ(getConsumersCount(queueManager), 1);
    }
    REQUIRE_EQ(getConsumersCount(queueManager), 1);

    {
        [[maybe_unused]] auto producer = queueManager.createProducer();
        REQUIRE_EQ(getConsumersCount(queueManager), 2);
    }

    REQUIRE_EQ(getConsumersCount(queueManager), 2);

    queueManager.forEachConsumer([&](LoggerQueue::Consumer* const consumer) {
        REQUIRE(consumer);
        REQUIRE(*consumer);
        consumer->close();
    });

    REQUIRE_EQ(getConsumersCount(queueManager), 0);
}

} // namespace rocket::logger::detail
