#pragma once

#include <cstddef>
#include <utility>

namespace smart_pointers {

// Simple default deleter
template <typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0, "T must be complete");
        delete ptr;
    }
};

// Partial specialization for arrays (T[])
template <typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) const noexcept {
        static_assert(sizeof(T) > 0, "T must be complete");
        delete[] ptr;
    }
};

template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    using pointer      = T*;
    using element_type = T;
    using deleter_type = Deleter;

    // ctors
    constexpr UniquePtr() noexcept = default;

    explicit UniquePtr(pointer ptr) noexcept
        : ptr_(ptr) {}

    UniquePtr(pointer ptr, const Deleter& d) noexcept
        : ptr_(ptr), deleter_(d) {}

    // non-copyable
    UniquePtr(const UniquePtr&)            = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;

    // movable
    UniquePtr(UniquePtr&& other) noexcept
        : ptr_(other.ptr_), deleter_(std::move(other.deleter_)) {
        other.ptr_ = nullptr;
    }

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) return *this;
        reset();
        ptr_      = other.ptr_;
        deleter_  = std::move(other.deleter_);
        other.ptr_ = nullptr;
        return *this;
    }

    ~UniquePtr() {
        reset();
    }

    // observers
    [[nodiscard]] pointer get() const noexcept { return ptr_; }
    [[nodiscard]] element_type& operator*() const noexcept { return *ptr_; }
    [[nodiscard]] pointer operator->() const noexcept { return ptr_; }
    [[nodiscard]] explicit operator bool() const noexcept { return ptr_ != nullptr; }

    deleter_type& get_deleter() noexcept { return deleter_; }
    const deleter_type& get_deleter() const noexcept { return deleter_; }

    // modifiers
    void reset(pointer new_ptr = pointer()) noexcept {
        if (ptr_ != new_ptr) {
            if (ptr_) {
                deleter_(ptr_);
            }
            ptr_ = new_ptr;
        }
    }

    [[nodiscard]] pointer release() noexcept {
        pointer tmp = ptr_;
        ptr_ = nullptr;
        return tmp;
    }

    void swap(UniquePtr& other) noexcept {
        using std::swap;
        swap(ptr_, other.ptr_);
        swap(deleter_, other.deleter_);
    }

private:
    pointer      ptr_{nullptr};
    deleter_type deleter_{};
};

// make_unique (single object)
template <typename T, typename... Args>
[[nodiscard]] UniquePtr<T> make_unique(Args&&... args) {
    return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

// make_unique for arrays
template <typename T>
[[nodiscard]] UniquePtr<T> make_unique_array(std::size_t n) {
    using Elem = std::remove_extent_t<T>;
    return UniquePtr<T>(new Elem[n]());
}

} // namespace smart_pointers

