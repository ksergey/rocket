// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include <doctest/doctest.h>

#include <ranges>

#include "Validator.h"

namespace rocket::config {

TEST_CASE("Validator: basic 0") {
    constexpr auto cmp = ge(5) && le(10);
    for (auto i : std::views::iota(0, 4)) {
        REQUIRE_EQ(cmp(i), false);
    }
    for (auto i : std::views::iota(5, 10)) {
        REQUIRE_EQ(cmp(i), true);
    }
    for (auto i : std::views::iota(11, 20)) {
        REQUIRE_EQ(cmp(i), false);
    }
}

TEST_CASE("Validator: basic 1") {
    using namespace std::string_view_literals;

    constexpr auto cmp = eq("hello");
    REQUIRE_EQ(cmp("hello"), true);
    REQUIRE_EQ(cmp("hello"sv), true);
    REQUIRE_EQ(cmp(""), false);
    REQUIRE_EQ(cmp("world"), false);
}

TEST_CASE("Validator: basic 2") {
    using namespace std::string_view_literals;

    constexpr auto cmp0 = empty();
    constexpr auto cmp1 = !empty();
    REQUIRE_EQ(cmp0("hello"), false);
    REQUIRE_EQ(cmp1("hello"), true);
    REQUIRE_EQ(cmp0(""sv), true);
    REQUIRE_EQ(cmp1(""sv), false);
    REQUIRE_EQ(cmp0(""), true);
    REQUIRE_EQ(cmp1(""), false);
}

TEST_CASE("Validator: oneOf") {
    using namespace std::string_view_literals;

    constexpr auto cmp = oneOf("debug"sv, "trace"sv, "info"sv, "error", "warning");
    REQUIRE_EQ(cmp("hello"), false);
    REQUIRE_EQ(cmp("debug"), true);
    REQUIRE_EQ(cmp("trace"), true);
    REQUIRE_EQ(cmp("info"), true);
    REQUIRE_EQ(cmp("error"), true);
    REQUIRE_EQ(cmp("warning"), true);
    REQUIRE_EQ(cmp("warning"sv), true);
    REQUIRE_EQ(cmp("war"sv), false);
}

TEST_CASE("Validator: pattern") {
    auto cmp = match(R"(^(?:[0-9]{1,3}\.){3}[0-9]{1,3}:\d{1,5}$)");

    REQUIRE_EQ(cmp("hello, world"), false);
    REQUIRE_EQ(cmp("1.1.1.1"), false);
    REQUIRE_EQ(cmp("1.1.1.1:31337"), true);
    REQUIRE_EQ(cmp("192.168.88.100:15"), true);
}

} // namespace rocket::config
