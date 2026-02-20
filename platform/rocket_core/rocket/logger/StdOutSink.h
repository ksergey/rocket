// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include "Sink.h"

namespace rocket::logger {

class StdOutSink final : public Sink {
  private:
    PatternFormatter formatter_;

  public:
    StdOutSink(StdOutSink const&) = delete;
    StdOutSink& operator=(StdOutSink const&) = delete;
    StdOutSink() = default;

    /// Formatting pattern
    [[nodiscard]] auto pattern() const noexcept -> std::string const& {
        return formatter_.pattern();
    }

    /// Set formatting pattern
    void setPattern(std::string value) noexcept {
        formatter_.setPattern(std::move(value));
    }

    void write(std::source_location const& location, LogLevel level, ::timespec const& timestamp,
        std::thread::id const& threadID, std::string_view message) override;

    void flush() override;
};

} // namespace rocket::logger
