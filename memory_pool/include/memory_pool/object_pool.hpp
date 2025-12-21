#pragma once

#include <cstddef>
#include <new>
#include <memory>
#include <vector>
#include <utility>
#include <type_traits>

namespace memory_pool {

template <typename T, std::size_t N>
class ObjectPool {
public:
    ObjectPool() {
        static_assert(N > 0, "ObjectPool capacity N must be > 0");
        freelist_.reserve(N);

        pool_ = static_cast<T*>(
            ::operator new(sizeof(T) * N, std::align_val_t{alignof(T)})
        );

        for (std::size_t i = 0; i < N; ++i) {
            freelist_.push_back(std::launder(pool_ + i));
        }
    }

    ~ObjectPool() {
        // We assume all objects have been returned.
        // If you want to be stricter, you can track live objects.
        ::operator delete(pool_, std::align_val_t{alignof(T)});
    }

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

    /// Allocate and construct a T with forwarded args.
    template <class... Args>
    [[nodiscard]] T* allocate(Args&&... args) {
        if (freelist_.empty()) {
            throw std::bad_alloc{};
        }

        T* slot = freelist_.back();
        freelist_.pop_back();

        try {
            return std::construct_at(slot, std::forward<Args>(args)...);
        } catch (...) {
            freelist_.push_back(slot);
            throw;
        }
    }

    /// Destroy and return the slot to the freelist.
    void deallocate(T* ptr) noexcept {
        if (!ptr) return;
        std::destroy_at(ptr);
        freelist_.push_back(ptr);
    }

    struct Deleter {
        ObjectPool* pool{};
        void operator()(T* ptr) const noexcept {
            if (pool && ptr) {
                pool->deallocate(ptr);
            }
        }
    };

    /// Make a unique_ptr that returns the object to this pool on destruction.
    template <class... Args>
    [[nodiscard]] std::unique_ptr<T, Deleter> make_unique(Args&&... args) {
        return std::unique_ptr<T, Deleter>(
            allocate(std::forward<Args>(args)...),
            Deleter{this}
        );
    }

    [[nodiscard]] std::size_t capacity() const noexcept { return N; }
    [[nodiscard]] std::size_t free_slots() const noexcept { return freelist_.size(); }

private:
    T* pool_{nullptr};
    std::vector<T*> freelist_;
};

} // namespace memory_pool

