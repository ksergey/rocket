// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "FileStream.h"

#include <fmt/format.h>

#include <system_error>

namespace rocket {

FileStream::FileStream(FileStream&& other) noexcept : file_(other.file_), owns_(other.owns_) {
    other.release();
}

FileStream& FileStream::operator=(FileStream&& other) noexcept {
    [[maybe_unused]] auto const result = closeNoThrow();
    swap(other);
    return *this;
}

FileStream::FileStream(std::filesystem::path const& path, char const* mode) {
    file_ = fopen(path.c_str(), mode);
    if (!file_) {
        throw std::system_error(errno, getPosixErrorCategory(), "fopen(...)");
    }
    owns_ = true;
}

FileStream::~FileStream() noexcept {
    if (auto const result = closeNoThrow(); !result) {
        fmt::print(stderr, "rocket: failed to close file stream ({})\n", result.error().message());
    }
}

auto FileStream::release() noexcept -> FILE* {
    FILE* released = file_;
    file_ = nullptr;
    owns_ = false;
    return released;
}

auto FileStream::closeNoThrow() noexcept -> std::expected<void, std::error_code> {
    int const rc = owns_ ? ::fclose(file_) : 0;
    release();
    if (rc != 0) {
        return std::unexpected(makePosixErrorCode(errno));
    } else {
        return {};
    }
}

void FileStream::close() {
    if (auto const result = closeNoThrow(); !result) {
        throw std::system_error(result.error(), "fclose(...)");
    }
}

} // namespace rocket
