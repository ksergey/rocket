// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "Sink.h"

#include <fmt/chrono.h>
#include <fmt/std.h>

namespace rocket::logger {
namespace {

// Wrapper around localtime_r
ROCKET_FORCE_INLINE auto localtime(std::time_t time) noexcept -> ::tm {
    ::tm tm;
    return *::localtime_r(&time, &tm);
}

} // namespace

auto PatternFormatter::operator()(std::source_location const& location, LogLevel level, ::timespec const& timestamp,
    std::thread::id const& threadID, std::string_view message) -> std::string_view {
    // serialize time
    char buffer[30];
    auto const result =
        fmt::format_to_n(buffer, sizeof(buffer), "{:%F %T}.{:09}", localtime(timestamp.tv_sec), timestamp.tv_nsec);
    auto const timestampStr = std::string_view(buffer, result.size);

    buffer_.clear();

    fmt::format_to(std::back_inserter(buffer_), fmt::runtime(pattern_), fmt::arg("timestamp", timestampStr),
        fmt::arg("threadID", threadID), fmt::arg("level", toShortString(level)), fmt::arg("message", message),
        fmt::arg("file", location.file_name()), fmt::arg("line", location.line()));

    return std::string_view{buffer_.data(), buffer_.size()};
}

} // namespace rocket::logger
