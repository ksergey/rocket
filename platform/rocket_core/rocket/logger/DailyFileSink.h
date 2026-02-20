// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <filesystem>

#include "../FileStream.h"
#include "Sink.h"

namespace rocket::logger {

class DailyFileSink final : public Sink {
  private:
    std::filesystem::path destination_;
    std::string prefix_;
    FileStream fileStream_;
    PatternFormatter formatter_;
    std::time_t nextRotateTime_ = 0;

  public:
    DailyFileSink(DailyFileSink const&) = delete;
    DailyFileSink& operator=(DailyFileSink const&) = delete;

    DailyFileSink(
        std::filesystem::path const& destination = std::filesystem::current_path(), std::string prefix = std::string());

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

  private:
    [[nodiscard]] auto reopen(std::time_t now) -> bool;
};

} // namespace rocket::logger
