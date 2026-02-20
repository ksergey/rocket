// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#include "BackendThread.h"

#include <ranges>

#include <fmt/format.h>

#include "../../LoopRateLimit.h"
#include "../../ThreadUtils.h"

namespace rocket::logger::detail {

BackendThread::BackendThread(LoggerQueueManager& queueManager) : queueManager_{queueManager} {}

BackendThread::~BackendThread() {
    this->stop();
}

void BackendThread::start(std::unique_ptr<Sink> sink, BackendOptions const& options) {
    assert(!this->isRunning() && "Backend already started");
    assert(sink.get() && "Invalid sink");

    auto thread = std::jthread([this, sink = std::move(sink), options] {
        if (options.bindToCoreNo) {
            if (auto const rc = pinCurrentThreadToCoreNo(*options.bindToCoreNo); rc) {
                fmt::print(stderr, "rocket: failed to pin backend thread to coreNo {}: {}\n", *options.bindToCoreNo,
                    rc.error().message());
            }
        }

        running_.store(true, std::memory_order_seq_cst);

        auto loopRateLimit = LoopRateLimit{options.sleepDuration};
        while (running_.load(std::memory_order_relaxed)) {
            try {
                this->processIncomingLogRecords(*sink);
            } catch (std::exception const& e) {
                fmt::print(stderr, "rocket: logger backend thread error: {}\n", e.what());
            }
            loopRateLimit.sleep();
        }

        while (processIncomingLogRecords(*sink) > 0) {}
    });

    thread_.swap(thread);

    // Wait until thread started
    while (!running_.load(std::memory_order_seq_cst)) {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void BackendThread::stop() {
    // Stop backend thread
    if (!running_.exchange(false)) {
        return;
    }
    // Join thread
    if (thread_.joinable()) {
        thread_.join();
    }
}

auto BackendThread::processIncomingLogRecords(Sink& sink) -> std::size_t {
    std::size_t count = 0;

    auto doFlush = false;

    queueManager_.forEachConsumer([&](LoggerQueue::Consumer* consumer) {
        // Dequeue all available messages
        while (true) {
            auto const success = consumer->dequeue([&](std::byte const* src) {
                ++count;
                auto const event = Codec<RecordHeader>::decode(src);

                switch (event.type) {
                case EventType::LogRecord: {
                    auto const logRecordHeader = Codec<LogRecordHeader>::decode(src);
                    auto const metadata = Codec<RecordMetadata*>::decode(src);
                    this->processLogRecord(sink, &logRecordHeader, metadata, src);
                    doFlush = true;
                } break;
                default: break;
                }
            });
            if (!success) {
                break;
            }
        }
    });

    if (doFlush) {
        sink.flush();
    }

    return count;
}

void BackendThread::processLogRecord(
    Sink& sink, LogRecordHeader const* logRecordHeader, RecordMetadata const* metadata, std::byte const* argsBuffer) {
    assert(logRecordHeader);
    assert(metadata);
    assert(argsBuffer);

    // Decode args for format
    fmt::dynamic_format_arg_store<fmt::format_context> store;
    metadata->decodeArgs(argsBuffer, &store);

    formatBuffer_.resize(0);
    fmt::vformat_to(std::back_inserter(formatBuffer_), metadata->format, store);
    auto const lines = std::string{formatBuffer_.data(), formatBuffer_.size()};

    constexpr auto kLineDelim = std::string_view{"\n"};
    for (auto const message : std::views::split(lines, kLineDelim)) {
        sink.write(*metadata->location, metadata->level, Clock::toTimeSpec(logRecordHeader->timestamp),
            logRecordHeader->threadID, std::string_view{message});
    }
}

} // namespace rocket::logger::detail
