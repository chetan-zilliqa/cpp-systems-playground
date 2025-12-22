#pragma once

#include <atomic>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace lock_free {

// Single-producer / single-consumer lock-free queue implemented as a ring buffer.
// Capacity is the maximum number of elements that can be in the queue at once.
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
    bool push(const T& value) {
        return emplace_impl(value);
    }

    // Move push (rvalue)
    bool push(T&& value) {
        return emplace_impl(std::move(value));
    }

    // Emplace in-place
    template <typename... Args>
    bool emplace(Args&&... args) {
        return emplace_impl(std::forward<Args>(args)...);
    }

    // Pop into out parameter. Returns false if queue is empty.
    bool pop(T& out) {
        auto tail = tail_.load(std::memory_order_relaxed);
        auto head = head_.load(std::memory_order_acquire);

        if (tail == head) {
            // empty
            return false;
        }

        T* elem = ptr(tail);
        out = std::move(*elem);
        elem->~T();

        auto next_tail = next_index(tail);
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }

    // Returns true if queue is empty (approximate in concurrent context).
    bool empty() const noexcept {
        auto tail = tail_.load(std::memory_order_relaxed);
        auto head = head_.load(std::memory_order_acquire);
        return tail == head;
    }

    // Returns true if queue is full (approximate).
    bool full() const noexcept {
        auto head      = head_.load(std::memory_order_relaxed);
        auto next_head = next_index(head);
        auto tail      = tail_.load(std::memory_order_acquire);
        return next_head == tail;
    }

    // Approximate size (not strictly accurate under concurrency, but fine for monitoring).
    std::size_t size() const noexcept {
        auto head = head_.load(std::memory_order_acquire);
        auto tail = tail_.load(std::memory_order_acquire);
        if (head >= tail) {
            return head - tail;
        }
        return BufferSize - (tail - head);
    }

    // Drain queue, destroying remaining elements.
    void clear() noexcept {
        T tmp;
        while (pop(tmp)) {
            // drain
        }
    }

private:
    template <typename... Args>
    bool emplace_impl(Args&&... args) {
        auto head      = head_.load(std::memory_order_relaxed);
        auto tail      = tail_.load(std::memory_order_acquire);
        auto next_head = next_index(head);

        if (next_head == tail) {
            // full
            return false;
        }

        T* elem = ptr(head);
        ::new (static_cast<void*>(elem)) T(std::forward<Args>(args)...);

        head_.store(next_head, std::memory_order_release);
        return true;
    }

    static std::size_t next_index(std::size_t i) noexcept {
        return (i + 1) % BufferSize;
    }

    T* ptr(std::size_t idx) noexcept {
        return std::launder(reinterpret_cast<T*>(&buffer_[idx]));
    }

    const T* ptr(std::size_t idx) const noexcept {
        return std::launder(reinterpret_cast<const T*>(&buffer_[idx]));
    }

    alignas(64) std::atomic<std::size_t> head_{0}; // producer index
    alignas(64) std::atomic<std::size_t> tail_{0}; // consumer index

    // Ring buffer storage
    alignas(64) Storage buffer_[BufferSize];
};

} // namespace lock_free

