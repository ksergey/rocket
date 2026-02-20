// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <doctest/doctest.h>

#include <chrono>
#include <ranges>
#include <thread>

#include "DailyFileSink.h"
#include "Logger.h"
#include "StdOutSink.h"

namespace rocket::logger {

TEST_CASE("Logger: ohm") {
    REQUIRE_FALSE(rocket::logger::isBackendReady());

    rocket::logger::setLogLevel("trace");

    rocket::logger::startBackend(std::make_unique<StdOutSink>());
    REQUIRE(rocket::logger::isBackendReady());

    REQUIRE_EQ(rocket::logger::logLevel(), LogLevel::Trace);

    logNotice("Hello {}!", "world");

    rocket::logger::stopBackend();
}

TEST_CASE("Logger: DailyFileSink") {
    REQUIRE_FALSE(rocket::logger::isBackendReady());

    rocket::logger::startBackend(std::make_unique<DailyFileSink>());
    REQUIRE(rocket::logger::isBackendReady());

    logWarningF("begin");
    for (auto i : std::views::iota(1) | std::views::take(50)) {
        logWarningF("record #{}", i);
    }
    logWarningF("end");

    std::jthread([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        logWarningF("begin");
        for (auto i : std::views::iota(50) | std::views::take(25)) {
            logWarningF("record #{}", i);
        }
        logWarningF("end");
    }).join();

    rocket::logger::stopBackend();
}

} // namespace rocket::logger
