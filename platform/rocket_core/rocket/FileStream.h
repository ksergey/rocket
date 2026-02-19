// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <cstdio>
#include <expected>
#include <filesystem>

#include "Platform.h"
#include "PosixError.h"

namespace rocket {

/// Wrapper around FILE*
class FileStream {
  private:
    FILE* file_ = nullptr;
    bool owns_ = false;

  public:
    FileStream(FileStream const&) = delete;
    FileStream& operator=(FileStream const&) = delete;

    FileStream(FileStream&& other) noexcept;
    FileStream& operator=(FileStream&& other) noexcept;

    FileStream() = default;

    /// Construct from raw file stream.
    /// Become owner on \c owns set to true.
    explicit FileStream(FILE* file, bool owns = false) noexcept : file_(file), owns_(owns) {}

    /// Open file stream
    FileStream(std::filesystem::path const& path, char const* mode);

    /// Destructor
    ~FileStream() noexcept;

    /// Implied cast to FILE*
    ROCKET_FORCE_INLINE operator FILE*() noexcept {
        return file_;
    }

    /// Returns true on FileStream initialized and could be used
    [[nodiscard]] ROCKET_FORCE_INLINE auto valid() const noexcept -> bool {
        return file_ != nullptr;
    }

    /// \see valid()
    [[nodiscard]] ROCKET_FORCE_INLINE explicit operator bool() const noexcept {
        return valid();
    }

    /// Returns and releases file descriptor
    auto release() noexcept -> FILE*;

    /// Close stream if owned
    auto closeNoThrow() noexcept -> std::expected<void, std::error_code>;

    /// Close stream if owned. Throws on error
    void close();

    /// Swap resources with other stream
    void swap(FileStream& other) noexcept {
        using std::swap;
        swap(file_, other.file_);
        swap(owns_, other.owns_);
    }

    /// Swaps file streams and ownership
    friend void swap(FileStream& lhs, FileStream& rhs) noexcept {
        lhs.swap(rhs);
    }
};

} // namespace rocket
