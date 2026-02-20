// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <fmt/format.h>

#include <string>
#include <string_view>

#include "Common.h"

namespace rocket::logger {

/// Logger sink interface
struct Sink {
    virtual ~Sink() = default;

    /// Write log entry
    virtual void write(std::source_location const& location, LogLevel level, ::timespec const& timestamp,
        std::thread::id const& threadID, std::string_view message) = 0;

    /// Flush
    virtual void flush() {}
};

/// Convert LogLevel value to readable string (short version)
constexpr auto toShortString(LogLevel level) noexcept -> std::string_view {
    using namespace std::string_view_literals;

    switch (level) {
    case LogLevel::Always: return "-"sv;
    case LogLevel::Error: return "E"sv;
    case LogLevel::Warning: return "W"sv;
    case LogLevel::Notice: return "I"sv;
    case LogLevel::Debug: return "D"sv;
    case LogLevel::Trace: return "T"sv;
    default: break;
    }

    return ""sv;
}

/// Pattern formatter
/// Available tokens:
///   - timestamp - timestamp in format YYYY-mm-dd HH:MM:SS.sssssssss
///   - level - log level as short string
///   - threadID - log source thread id
///   - message - formatted message
///   - file - full path to source file
///   - line - line at source file
class PatternFormatter {
  private:
    std::string pattern_ = "{timestamp} [{level}] ({threadID}) {message} ({file}:{line})";
    fmt::memory_buffer buffer_;

  public:
    PatternFormatter() = default;

    /// Current pattern
    [[nodiscard]] auto pattern() const noexcept -> std::string const& {
        return pattern_;
    }

    /// Change pattern
    void setPattern(std::string value) noexcept {
        pattern_ = std::move(value);
    }

    /// Format string
    /// Result valid until next call
    [[nodiscard]] auto operator()(std::source_location const& location, LogLevel level, ::timespec const& timestamp,
        std::thread::id const& threadID, std::string_view message) -> std::string_view;
};

} // namespace rocket::logger
