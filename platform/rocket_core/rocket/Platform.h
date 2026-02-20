// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <cstddef>
#include <new>

#ifndef ROCKET_FORCE_INLINE
#define ROCKET_FORCE_INLINE inline __attribute__((always_inline))
#endif

#ifndef ROCKET_NO_INLINE
#define ROCKET_NO_INLINE inline __attribute__((noinline))
#endif

#ifndef ROCKET_COLD
#define ROCKET_COLD __attribute__((cold))
#endif

namespace rocket {

/// Mimic: std::hardware_destructive_interference_size
constexpr std::size_t kHardwareDestructiveInterferenceSize = 64;

/// Mimic: std::hardware_constructive_interference_size
constexpr std::size_t kHardwareConstructiveInterferenceSize = 64;

} // namespace rocket
