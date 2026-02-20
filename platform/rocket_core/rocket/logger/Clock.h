// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <cstdint>
#include <ctime>

#include "../Platform.h"

namespace rocket::logger {

/// Clock for timestamps
template <clockid_t ClockID>
struct LinuxClock {
    using Timestamp = ::timespec;

    [[nodiscard]] ROCKET_FORCE_INLINE static auto now() noexcept -> Timestamp {
        timespec ts;
        ::clock_gettime(ClockID, &ts);
        return ts;
    }

    [[nodiscard]] static constexpr auto toTimeSpec(::timespec const& value) noexcept -> ::timespec const& {
        return value;
    }
};

struct TSCClock {
    using Timestamp = std::int64_t;

    [[nodiscard]] ROCKET_FORCE_INLINE static auto now() noexcept -> Timestamp {
        return __builtin_ia32_rdtsc();
    }

    [[nodiscard]] static auto toTimeSpec(std::int64_t value) noexcept -> ::timespec;
};

#if defined(ROCKET_LOGGER_TSC_CLOCK)
using Clock = TSCClock;
#else
using Clock = LinuxClock<CLOCK_REALTIME>;
#endif

} // namespace rocket::logger
