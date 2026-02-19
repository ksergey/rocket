// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <doctest/doctest.h>

#include <ranges>

#include <fmt/format.h>

#include "LoopRateLimit.h"

namespace rocket {

TEST_CASE("LoopRateLimit") {

    auto loopRateLimit = LoopRateLimit(std::chrono::milliseconds(500));

    for (auto const i : std::views::iota(1) | std::views::take(10)) {
        fmt::print(stdout, "iteration #{}\n", i);
        loopRateLimit.sleep();
    }
}

} // namespace rocket
