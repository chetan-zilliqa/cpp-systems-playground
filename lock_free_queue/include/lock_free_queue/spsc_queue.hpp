#pragma once

#include <atomic>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace lock_free {

/**
 * Single-producer / single-consumer lock-free queue implemented as a ring buffer.
 * 
 * DESIGN:
 * - Uses a fixed-size circular buffer with head (producer) and tail (consumer) indices
 * - Both indices are atomic to enable wait-free operation
 * - Capacity+1 slots distinguish between empty (head==tail) and full (head+1==tail mod size)
 * 
 * MEMORY ORDERING:
 * - head: modified only by producer, read by both
 * - tail: modified only by consumer, read by both
 * - Cache-line aligned (64 bytes) to avoid false sharing
 * 
 * MEMORY SEMANTICS:
 * - emplace: load head (relaxed), load tail (acquire), store head (release)
 *   → Producer ordered against consumer reads of head
 * - pop: load tail (relaxed), load head (acquire), store tail (release)
 *   → Consumer ordered against producer reads of tail
 * 
 * LIMITATIONS:
 * - Single producer, single consumer only
 * - Fixed capacity (no dynamic resizing)
 * - For MPMC, see mpmc_queue.hpp
 * 
 * THREAD SAFETY:
 * - Wait-free: no loops, allocations, or blocking operations
 * - No spurious wakeups or conditional variables
 * - Safe under single producer, single consumer constraint
 */
template <typename T, std::size_t Capacity>
class SPSCQueue {
    static_assert(Capacity >= 1, "Capacity must be at least 1");

    // We use Capacity + 1 slots to distinguish full vs empty
    static constexpr std::size_t BufferSize = Capacity + 1;

    using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;

public:
    SPSCQueue() noexcept = default;

    SPSCQueue(const SPSCQueue&)            = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    ~SPSCQueue() {
        clear();
    }

    // Non-copying push (lvalue)
    // Producer only. Returns true if item was enqueued, false if full.
    bool push(const T& value) {
        return emplace_impl(value);
    }

    // Move push (rvalue)
    // Producer only. Returns true if item was enqueued, false if full.
    bool push(T&& value) {
        return emplace_impl(std::move(value));
    }

    // Emplace in-place with perfect forwarding
    // Producer only. Constructs T directly in queue storage.
    // Returns true if item was enqueued, false if full.
    template <typename... Args>
    bool emplace(Args&&... args) {
        return emplace_impl(std::forward<Args>(args)...);
    }

    // Pop front element (consumer side)
    // Consumer only. Moves element out to 'out' parameter.
    // Returns false if queue is empty.
    bool pop(T& out) {
        auto tail = tail_.load(std::memory_order_relaxed);
        auto head = head_.load(std::memory_order_acquire);

        if (tail == head) {
            // empty - no items to consume
            return false;
        }

        T* elem = ptr(tail);
        out = std::move(*elem);
        elem->~T();  // explicitly call destructor

        auto next_tail = next_index(tail);
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    // Returns true if queue is empty (approximate in concurrent context).
    // Safe to call from either producer or consumer, but not precise under contention.
    bool empty() const noexcept {
        auto tail = tail_.load(std::memory_order_relaxed);
        auto head = head_.load(std::memory_order_acquire);
        return tail == head;
    }

    // Returns true if queue is full (approximate).
    // Safe to call from either producer or consumer, but not precise under contention.
    bool full() const noexcept {
        auto head      = head_.load(std::memory_order_relaxed);
        auto next_head = next_index(head);
        auto tail      = tail_.load(std::memory_order_acquire);
        return next_head == tail;
    }

    // Approximate size (not strictly accurate under concurrency, but fine for monitoring).
    // Handles wrap-around arithmetic correctly.
    std::size_t size() const noexcept {
        auto head = head_.load(std::memory_order_acquire);
        auto tail = tail_.load(std::memory_order_acquire);
        if (head >= tail) {
            return head - tail;
        }
        return BufferSize - (tail - head);
    }

    // Drain queue, destroying remaining elements.
    // Should only be called when guaranteed no producer/consumer access.
    void clear() noexcept {
        T tmp;
        while (pop(tmp)) {
            // drain
        }
    }

private:
    // Internal emplace implementation (used by push and emplace)
    // Atomically reserves head slot and constructs T in-place.
    // Uses acquire-release semantics:
    //   - Load tail with acquire to ensure memory ordering
    //   - Store head with release to publish the new element
    template <typename... Args>
    bool emplace_impl(Args&&... args) {
        auto head      = head_.load(std::memory_order_relaxed);
        auto tail      = tail_.load(std::memory_order_acquire);
        auto next_head = next_index(head);

        if (next_head == tail) {
            // full - can't enqueue
            return false;
        }

        T* elem = ptr(head);
        ::new (static_cast<void*>(elem)) T(std::forward<Args>(args)...);  // placement new

        head_.store(next_head, std::memory_order_release);
        return true;
    }

    // Circular buffer index increment (with wrap-around)
    static std::size_t next_index(std::size_t i) noexcept {
        return (i + 1) % BufferSize;
    }

    // Unsafe cast from storage buffer to typed pointer (use with placement new/delete)
    T* ptr(std::size_t idx) noexcept {
        return std::launder(reinterpret_cast<T*>(&buffer_[idx]));
    }

    // Const version for safe reads
    const T* ptr(std::size_t idx) const noexcept {
        return std::launder(reinterpret_cast<const T*>(&buffer_[idx]));
    }

    // Producer head index (cache-line aligned to avoid false sharing with tail)
    // Only modified by producer, read by consumer
    alignas(64) std::atomic<std::size_t> head_{0};
    
    // Consumer tail index (cache-line aligned)
    // Only modified by consumer, read by producer
    alignas(64) std::atomic<std::size_t> tail_{0};

    // Ring buffer storage (cache-line aligned)
    alignas(64) Storage buffer_[BufferSize];
};

} // namespace lock_free

