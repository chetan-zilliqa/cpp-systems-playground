#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace memory_pool {

class FixedBlockMemoryPool {
public:
    FixedBlockMemoryPool(std::size_t block_size, std::size_t capacity)
        : block_size_(align_block_size(block_size))
        , capacity_(capacity)
        , buffer_(nullptr)
        , free_list_(nullptr)
    {
        if (capacity_ == 0) {
            throw std::invalid_argument("capacity must be > 0");
        }

        const std::size_t total_bytes = block_size_ * capacity_;
        buffer_ = static_cast<std::byte*>(::operator new(total_bytes));

        for (std::size_t i = 0; i < capacity_; ++i) {
            std::byte* block = buffer_ + i * block_size_;
            void* next = (i + 1 < capacity_)
                ? static_cast<void*>(buffer_ + (i + 1) * block_size_)
                : nullptr;

            *reinterpret_cast<void**>(block) = next;
        }

        free_list_ = buffer_;
    }

    ~FixedBlockMemoryPool() {
        ::operator delete(buffer_);
    }

    FixedBlockMemoryPool(const FixedBlockMemoryPool&) = delete;
    FixedBlockMemoryPool& operator=(const FixedBlockMemoryPool&) = delete;
    FixedBlockMemoryPool(FixedBlockMemoryPool&&) = delete;
    FixedBlockMemoryPool& operator=(FixedBlockMemoryPool&&) = delete;

    void* allocate() {
        if (!free_list_) {
            throw std::bad_alloc();
        }
        void* block = free_list_;
        free_list_ = *reinterpret_cast<void**>(free_list_);
        return block;
    }

    void deallocate(void* ptr) {
        if (!ptr) return;
        *reinterpret_cast<void**>(ptr) = free_list_;
        free_list_ = ptr;
    }

    std::size_t block_size() const noexcept { return block_size_; }
    std::size_t capacity() const noexcept { return capacity_; }

private:
    static std::size_t align_block_size(std::size_t sz) {
        constexpr std::size_t align = alignof(void*);
        return (sz + align - 1) & ~(align - 1);
    }

    std::size_t block_size_;
    std::size_t capacity_;
    std::byte* buffer_;
    void* free_list_;
};

} // namespace memory_pool

