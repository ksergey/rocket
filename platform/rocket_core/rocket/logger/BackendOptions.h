// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

namespace rocket::logger {

struct BackendOptions {
    /// Bind backend thread to a specified core
    std::optional<std::uint16_t> bindToCoreNo = std::nullopt;

    /// Sleep duration if there is no remaining work to process
    std::chrono::milliseconds sleepDuration = std::chrono::milliseconds{100};
};

} // namespace rocket::logger
