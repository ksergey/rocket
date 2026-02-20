// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "DailyFileSink.h"

#include <ranges>

#include <fmt/chrono.h>
#include <fmt/format.h>

namespace rocket::logger {
namespace {

auto calcNextRotateTime(std::time_t now) noexcept -> std::time_t {
    auto tm = *std::localtime(&now);
    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 0;
    return std::mktime(&tm) + 24 * 60 * 60;
}

} // namespace

DailyFileSink::DailyFileSink(std::filesystem::path const& destination, std::string prefix)
    : destination_(std::filesystem::canonical(destination)), prefix_(std::move(prefix)) {
    std::filesystem::create_directories(absolute(destination_));
    if (!prefix_.empty()) {
        prefix_ += "_";
    }
}

void DailyFileSink::write(std::source_location const& location, LogLevel level, ::timespec const& timestamp,
    std::thread::id const& threadID, std::string_view message) {
    auto const now = std::time_t(timestamp.tv_sec);
    if (nextRotateTime_ < now || !fileStream_) [[unlikely]] {
        if (!this->reopen(now)) {
            return;
        }
        nextRotateTime_ = calcNextRotateTime(now);
    }

    auto const formattedMessage = formatter_(location, level, timestamp, threadID, message);
    fmt::print(fileStream_, "{}\n", formattedMessage);
}

void DailyFileSink::flush() {
    if (fileStream_) [[likely]] {
        std::fflush(fileStream_);
    }
}

[[nodiscard]] auto DailyFileSink::reopen(std::time_t now) -> bool {
    // trim now to start of day at localtime
    ::tm tm;
    ::localtime_r(&now, &tm);
    tm.tm_sec = 0;
    tm.tm_min = 0;
    tm.tm_hour = 0;

    auto const path = [&]() -> std::filesystem::path {
        for (auto index : std::views::iota(0) | std::views::take(9999)) {
            auto filename = fmt::format("{}{:%Y%m%d}.{:04}.log", prefix_, tm, index);
            auto path = destination_ / filename;
            if (std::filesystem::exists(path)) {
                continue;
            }
            return path;
        }
        return {};
    }();

    auto file = std::fopen(path.native().c_str(), "a");
    if (!file) {
        return false;
    }
    fileStream_ = FileStream(file, true);
    return true;
}

} // namespace rocket::logger
