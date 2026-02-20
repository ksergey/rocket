// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "StdOutSink.h"

#include <ctime>

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/std.h>

namespace rocket::logger {

void StdOutSink::write(std::source_location const& location, LogLevel level, ::timespec const& timestamp,
    std::thread::id const& threadID, std::string_view message) {

    auto const style = [&]() noexcept -> fmt::text_style {
        switch (level) {
        case LogLevel::Error: return fg(fmt::color::red);
        case LogLevel::Warning: return fg(fmt::color::orange);
        case LogLevel::Debug: return fg(fmt::color::dim_gray);
        case LogLevel::Trace: return fg(fmt::color::dim_gray);
        default: break;
        }
        return fmt::text_style();
    }();

    auto const formattedMessage = formatter_(location, level, timestamp, threadID, message);
    fmt::print(style, "{}\n", formattedMessage);
}

void StdOutSink::flush() {
    std::fflush(stdout);
}

} // namespace rocket::logger
