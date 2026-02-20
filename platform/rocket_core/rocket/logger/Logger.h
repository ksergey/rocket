// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <string_view>

#include <fmt/format.h>

#include "Codec.h"
#include "Macro.h"
#include "Transform.h"
#include "detail/Backend.h"

namespace rocket::logger {

[[nodiscard]] ROCKET_FORCE_INLINE auto backend() -> detail::Backend* {
    return detail::Backend::instance();
}

namespace detail {

template <typename... Args>
constexpr void decodeFormatArgs(
    [[maybe_unused]] std::byte const* src, [[maybe_unused]] fmt::dynamic_format_arg_store<fmt::format_context>* store) {
    (store->push_back(Codec<Args>::decode(src)), ...);
}

} // namespace detail

/// Log verbosity level
ROCKET_FORCE_INLINE auto logLevel() noexcept -> LogLevel {
    return backend()->logLevel();
}

/// Set log verbosity level
ROCKET_FORCE_INLINE void setLogLevel(LogLevel value) {
    backend()->setLogLevel(value);
}

/// Return true on message with log level should be logged
ROCKET_FORCE_INLINE auto shouldLog(LogLevel value) noexcept -> bool {
    return backend()->shouldLog(value);
}

/// Set log verbosity level from string value
/// throws on error
ROCKET_FORCE_INLINE void setLogLevel(std::string_view value) {
    using namespace std::string_view_literals;

    if (value == "error"sv) {
        return backend()->setLogLevel(LogLevel::Error);
    } else if (value == "warning"sv) {
        return backend()->setLogLevel(LogLevel::Warning);
    } else if (value == "notice"sv) {
        return backend()->setLogLevel(LogLevel::Notice);
    } else if (value == "debug"sv) {
        return backend()->setLogLevel(LogLevel::Debug);
    } else if (value == "trace"sv) {
        return backend()->setLogLevel(LogLevel::Trace);
    }

    throw std::invalid_argument("invalid log level string value");
}

/// Queue capacity hint
[[nodiscard]] ROCKET_FORCE_INLINE auto queueCapacityHint() noexcept -> std::size_t {
    return backend()->queueCapacityHint();
}

/// Set queue capacity hint
ROCKET_FORCE_INLINE void setQueueCapacityHint(std::size_t sizeHint) {
    backend()->setQueueCapacityHint(sizeHint);
}

/// Check backend thread running
ROCKET_FORCE_INLINE auto isBackendReady() noexcept -> bool {
    return backend()->isReady();
}

/// Start backend thread
ROCKET_FORCE_INLINE void startBackend(std::unique_ptr<Sink> sink = {}, BackendOptions const& options = {}) {
    backend()->start(std::move(sink), options);
}

/// Stop backend thread
ROCKET_FORCE_INLINE void stopBackend() {
    backend()->stop();
}

/// Log statement handler
/// @tparam M is struct with metadata from macro
/// @param[in] args is arguments for log record
template <typename M, typename... Args>
ROCKET_FORCE_INLINE void logStatement(Args const&... args) {
    // compile time format string check
    [[maybe_unused]] static constexpr auto formatStringCheck = fmt::format_string<Args...>(M::format());

    auto const now = Clock::now();

    // clang-format off
  [[maybe_unused]] static constexpr auto meta = RecordMetadata{
      .location = M::location(),
      .level = M::level(),
      .format = M::format(),
      .flags = M::flags(),
      .decodeArgs = detail::decodeFormatArgs<Args...>
  };
    // clang-format on

    std::size_t const bufferSize = Codec<RecordHeader>::encodedSize() + Codec<LogRecordHeader>::encodedSize() +
                                   Codec<RecordMetadata const*>::encodedSize(&meta) +
                                   (0 + ... + Codec<Args>::encodedSize(args));

    constexpr auto kEnqueuePolicy =
        (kFlagRety == (meta.flags & kFlagRety)) ? detail::EnqueuePolicy::Retry : detail::EnqueuePolicy::Drop;

    auto const threadContext = backend()->localThreadContext();

    threadContext->producer().enqueue<kEnqueuePolicy>(bufferSize, [&](std::byte* dst) noexcept {
        // RecordHeader
        Codec<RecordHeader>::encode(dst, RecordHeader{.type = EventType::LogRecord});
        // LogRecordHeader
        Codec<LogRecordHeader>::encode(dst, LogRecordHeader{.timestamp = now, .threadID = threadContext->threadID()});
        // RecordMetadata
        Codec<RecordMetadata const*>::encode(dst, &meta);
        // Args...
        (Codec<Args>::encode(dst, args), ...);
    });

    // TODO: create new queue with doubled capacity on failed to enqueue?
}

/// Transform types into loggable values and pass to log(...)
template <typename M, typename... Args>
ROCKET_FORCE_INLINE void logStatementTransform(Args const&... args) {
    return logStatement<M>(transform(args)...);
}

} // namespace rocket::logger
