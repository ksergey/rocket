// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <source_location>
#include <string_view>
#include <thread>
#include <type_traits>

#include <fmt/args.h>

#include "../Platform.h"
#include "Clock.h"

namespace rocket::logger {

/// Log entry verbosity level
enum class LogLevel { Always, Error, Warning, Notice, Debug, Trace };

/// Decode args function signature
using DecodeArgsFn = std::add_pointer_t<void(std::byte const*, fmt::dynamic_format_arg_store<fmt::format_context>*)>;

/// Decode args for log entry from a buffer
using FormatFn = std::add_pointer_t<std::string(std::byte const*)>;

/// Flag indicates a record should never dropped at enqueue side
constexpr auto kFlagRety = int(1 << 0);

/// Log record metadata
struct RecordMetadata {
    /// Log entry source location
    std::source_location const* location;
    /// Log entry verbosity level
    LogLevel level;
    /// Format string
    std::string_view format;
    /// Log entry flags
    int flags;
    /// Function to decode log entry args from a buffer
    DecodeArgsFn decodeArgs;
};
static_assert(std::is_trivially_copyable_v<RecordMetadata>);

/// Event type
///
/// @c EventType::LogRecord
///   layout: RecordHeader{ .type = EventType::LogRecord } | LogRecordHeader | RecordMetadata* | Args...
enum EventType { LogRecord };

/// Log event header
struct RecordHeader {
    EventType type;
};
static_assert(std::is_trivially_copyable_v<RecordHeader>);

/// Log record header
struct LogRecordHeader {
    /// Log record timestamp
    Clock::Timestamp timestamp;
    /// Log record thread
    std::thread::id threadID;
};
static_assert(std::is_trivially_copyable_v<LogRecordHeader>);

} // namespace rocket::logger
