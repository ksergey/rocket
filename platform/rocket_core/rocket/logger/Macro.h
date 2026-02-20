// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#define ROCKET_LOG_CALL(LEVEL, FLAGS, FMT, ...)                                                                        \
    do {                                                                                                               \
        static constexpr auto thisSourceLocation = std::source_location::current();                                    \
        struct {                                                                                                       \
            static constexpr auto location() noexcept -> std::source_location const* {                                 \
                return &thisSourceLocation;                                                                            \
            }                                                                                                          \
            static constexpr auto level() noexcept -> ::rocket::logger::LogLevel {                                     \
                return ::rocket::logger::LogLevel::LEVEL;                                                              \
            }                                                                                                          \
            static constexpr auto flags() noexcept -> int {                                                            \
                return FLAGS;                                                                                          \
            }                                                                                                          \
            static constexpr auto format() noexcept -> std::string_view {                                              \
                return std::string_view{FMT};                                                                          \
            }                                                                                                          \
        } thisMeta;                                                                                                    \
        ::rocket::logger::logStatementTransform<decltype(thisMeta)>(__VA_ARGS__);                                      \
    } while (0)

#define ROCKET_LOG(LEVEL, FLAGS, FMT, ...)                                                                             \
    do {                                                                                                               \
        if (::rocket::logger::shouldLog(::rocket::logger::LogLevel::LEVEL)) {                                          \
            ROCKET_LOG_CALL(LEVEL, FLAGS, FMT, __VA_ARGS__);                                                           \
        }                                                                                                              \
    } while (0)

#define logAlways(...) ROCKET_LOG(Always, 0, __VA_ARGS__)
#define logError(...) ROCKET_LOG(Error, 0, __VA_ARGS__)
#define logWarning(...) ROCKET_LOG(Warning, 0, __VA_ARGS__)
#define logNotice(...) ROCKET_LOG(Notice, 0, __VA_ARGS__)
#define logDebug(...) ROCKET_LOG(Debug, 0, __VA_ARGS__)
#define logTrace(...) ROCKET_LOG(Trace, 0, __VA_ARGS__)

#define logAlwaysF(...) ROCKET_LOG(Always, ::rocket::logger::kFlagRety, __VA_ARGS__)
#define logErrorF(...) ROCKET_LOG(Error, ::rocket::logger::kFlagRety, __VA_ARGS__)
#define logWarningF(...) ROCKET_LOG(Warning, ::rocket::logger::kFlagRety, __VA_ARGS__)
#define logNoticeF(...) ROCKET_LOG(Notice, ::rocket::logger::kFlagRety, __VA_ARGS__)
#define logDebugF(...) ROCKET_LOG(Debug, ::rocket::logger::kFlagRety, __VA_ARGS__)
#define logTraceF(...) ROCKET_LOG(Trace, ::rocket::logger::kFlagRety, __VA_ARGS__)
