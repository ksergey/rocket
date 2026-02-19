// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <cstdint>
#include <expected>
#include <string>

#include "PosixError.h"

namespace rocket {

/// Return name of the thread
auto getThisThreadName() -> std::expected<std::string, std::error_code>;

/// Set name of the thread
auto setThisThreadName(std::string const& value) -> std::expected<void, std::error_code>;

/// Pin current thread to specified core number
auto pinCurrentThreadToCoreNo(std::uint16_t coreNo) noexcept -> std::expected<void, std::error_code>;

} // namespace rocket
