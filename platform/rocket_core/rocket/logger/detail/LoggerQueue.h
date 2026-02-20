// Copyright (c) Sergey Kovalevich <inndie@gmail.com>
// SPDX-License-Identifier: AGPL-3.0

#pragma once

#include <immintrin.h>

#include <functional>
#include <string_view>
#include <tuple>

#include <fmt/format.h>

#include "../../Platform.h"
#include "../../TypeTraits.h"
#include "../Codec.h"
#include "../Common.h"
#include "BoundedSPSCRawQueue.h"

namespace rocket::logger::detail {

/// Mimic: std::this_thread::yield()
ROCKET_FORCE_INLINE void yield() noexcept {
    _mm_pause(); // or std::this_thread::yield();
}

/// Enqueue policy
enum class EnqueuePolicy {
    Drop, ///< Drop on write into queue failed
    Retry ///< Spin a bit and try again
};

struct LoggerQueue {
    /// Queue producer
    struct Producer : public BoundedSPSCRawQueue::Producer {
        using BoundedSPSCRawQueue::Producer::Producer;

        Producer(BoundedSPSCRawQueue::Producer producer) noexcept
            : BoundedSPSCRawQueue::Producer(std::move(producer)) {}

        /// Enqueue a data into queue
        template <EnqueuePolicy Policy = EnqueuePolicy::Drop, typename Fn>
        ROCKET_FORCE_INLINE auto enqueue(std::size_t size, Fn&& fn) -> bool {
            auto buffer = this->prepare(size);

            if constexpr (Policy == EnqueuePolicy::Drop) {
                if (buffer.empty()) [[unlikely]] {
                    return false;
                }
            } else if constexpr (Policy == EnqueuePolicy::Retry) {
                if (buffer.empty()) [[unlikely]] {
                    do {
                        yield();
                        buffer = this->prepare(size);
                    } while (buffer.empty());
                }
            } else {
                static_assert(FalseV<Policy>, "Unknown EnqueuePolicy value");
            }

            std::invoke(std::forward<Fn>(fn), buffer.data());
            this->commit();
            return true;
        }
    };

    /// Queue consumer
    struct Consumer : public BoundedSPSCRawQueue::Consumer {
        using BoundedSPSCRawQueue::Consumer::Consumer;

        Consumer(BoundedSPSCRawQueue::Consumer consumer) noexcept
            : BoundedSPSCRawQueue::Consumer(std::move(consumer)) {}

        template <typename Fn>
        ROCKET_FORCE_INLINE auto dequeue(Fn&& fn) -> bool {
            auto buffer = this->fetch();
            if (buffer.empty()) [[unlikely]] {
                return false;
            }
            std::invoke(std::forward<Fn>(fn), buffer.data());
            this->consume();
            return true;
        }
    };

    /// Create producer and consumer
    /// @param[in] name is queue name
    /// @param[in] capacityHint is queue capacity hint
    /// @return tuple with valid producer and consumer on success
    [[nodiscard]] static auto createProducerAndConsumer(std::string_view name, std::size_t capacityHint) noexcept
        -> std::tuple<Producer, Consumer> {
        try {
            auto const options = BoundedSPSCRawQueue::CreationOptions{.capacityHint = capacityHint};
            auto queue = BoundedSPSCRawQueue(name, options);
            return std::make_tuple<Producer, Consumer>(queue.createProducer(), queue.createConsumer());
        } catch (std::exception const& e) {
            fmt::print(stderr, "failed to create producer and consumer: {}\n", e.what());
        }
        return std::make_tuple(Producer(), Consumer());
    }
};

} // namespace rocket::logger::detail
