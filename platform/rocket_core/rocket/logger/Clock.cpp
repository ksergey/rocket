// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "Clock.h"

#include "detail/TicksHelper.h"

namespace rocket::logger {

auto TSCClock::toTimeSpec(std::int64_t value) noexcept -> ::timespec {
    static constexpr auto kNanosecondsInSecond =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)).count();
    auto const nsSinceEpoch = detail::TicksHelper::instance()->timeSinceEpoch(value);
    return ::timespec{.tv_sec = nsSinceEpoch / kNanosecondsInSecond, .tv_nsec = nsSinceEpoch % kNanosecondsInSecond};
}

} // namespace rocket::logger
