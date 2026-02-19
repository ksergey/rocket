// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

namespace rocket {

/// Evaluate any type argument into false value
/// Usage example:
///
/// ...
/// if constexpr (requires { run(T()); }) {
/// } else {
///   static_assert(False<T>, "no value for type T");
/// }
/// ...
template <typename>
constexpr bool False = false;

/// Same as False<typename> but for values
template <auto>
constexpr bool FalseV = false;

} // namespace rocket
