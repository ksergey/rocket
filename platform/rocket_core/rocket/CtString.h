// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <array>
#include <string_view>

namespace rocket {

/// Compile-time string
template <std::size_t N>
struct CtString {
    std::array<char, N> data{};

    /// Constructor: construct empty string
    consteval CtString() = default;

    /// Constructor: construct for c-string
    consteval CtString(char const* str) noexcept {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] = str[i];
        }
    }

    /// Constructor: construct from str and size
    consteval explicit CtString(char const* str, std::size_t size) noexcept {
        for (std::size_t i = 0; i < size; ++i) {
            data[i] = str[i];
        }
    }

    /// Constructor: from std::string_view
    constexpr explicit CtString(std::string_view str) noexcept : CtString(str.data(), str.size()) {}

    /// Implied convert to std::string_view
    [[nodiscard]] constexpr operator std::string_view() const noexcept {
        return std::string_view(data.data(), data.size());
    }
};

/// CTAD
template <std::size_t N>
CtString(char const (&)[N]) -> CtString<N - 1>;

} // namespace rocket
