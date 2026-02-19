// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <doctest/doctest.h>

#include "ThreadUtils.h"

namespace rocket {

TEST_CASE("Thread Name") {
    REQUIRE(setThisThreadName("SuperRipper").has_value());
    auto const rc = getThisThreadName();
    REQUIRE(rc.has_value());
    REQUIRE(rc.value() == "SuperRipper");
}

} // namespace rocket
