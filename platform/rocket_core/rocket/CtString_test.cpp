// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <doctest/doctest.h>

#include "CtString.h"

namespace rocket::core {

template <CtString S>
struct Str {
    static constexpr auto value = static_cast<std::string_view>(S);
};

TEST_CASE("CtString") {
    REQUIRE_EQ(Str<"Hello">::value, "Hello");
}

} // namespace rocket::core
