#pragma once

#include <cstddef>
#include <utility>

namespace smart_pointers {

// Simple non-thread-safe control block
template <typename T>
struct ControlBlock {
    T*        ptr;
    std::size_t strong_count;
    std::size_t weak_count;

    explicit ControlBlock(T* p)
        : ptr(p), strong_count(1), weak_count(0) {}
};

template <typename T> class SharedPtr;
template <typename T> class WeakPtr;

template <typename T>
class SharedPtr {
public:
    using element_type = T;

    // ctors
    constexpr SharedPtr() noexcept = default;

    explicit SharedPtr(T* ptr)
        : ctrl_(nullptr), ptr_(ptr) {
        if (ptr_) {
            ctrl_ = new ControlBlock<T>(ptr_);
        }
    }

    // copy
    SharedPtr(const SharedPtr& other) noexcept
        : ctrl_(other.ctrl_), ptr_(other.ptr_) {
        inc_strong();
    }

    SharedPtr& operator=(const SharedPtr& other) noexcept {
        if (this == &other) return *this;
        release();
        ctrl_ = other.ctrl_;
        ptr_  = other.ptr_;
        inc_strong();
        return *this;
    }

    // move
    SharedPtr(SharedPtr&& other) noexcept
        : ctrl_(other.ctrl_), ptr_(other.ptr_) {
        other.ctrl_ = nullptr;
        other.ptr_  = nullptr;
    }

    SharedPtr& operator=(SharedPtr&& other) noexcept {
        if (this == &other) return *this;
        release();
        ctrl_      = other.ctrl_;
        ptr_       = other.ptr_;
        other.ctrl_ = nullptr;
        other.ptr_  = nullptr;
        return *this;
    }

    ~SharedPtr() {
        release();
    }

    // observers
    [[nodiscard]] T* get() const noexcept { return ptr_; }
    [[nodiscard]] T& operator*() const noexcept { return *ptr_; }
    [[nodiscard]] T* operator->() const noexcept { return ptr_; }
    [[nodiscard]] explicit operator bool() const noexcept { return ptr_ != nullptr; }

    [[nodiscard]] std::size_t use_count() const noexcept {
        return ctrl_ ? ctrl_->strong_count : 0;
    }

    // modifiers
    void reset() noexcept {
        release();
    }

    void reset(T* new_ptr) {
        release();
        if (new_ptr) {
            ctrl_ = new ControlBlock<T>(new_ptr);
            ptr_  = new_ptr;
        }
    }

    void swap(SharedPtr& other) noexcept {
        using std::swap;
        swap(ctrl_, other.ctrl_);
        swap(ptr_, other.ptr_);
    }

private:
    friend class WeakPtr<T>;

    explicit SharedPtr(ControlBlock<T>* ctrl) noexcept
        : ctrl_(ctrl), ptr_(ctrl ? ctrl->ptr : nullptr) {
        inc_strong();
    }

    void inc_strong() noexcept {
        if (ctrl_) {
            ++ctrl_->strong_count;
        }
    }

    void release() noexcept {
        if (!ctrl_) return;

        if (--ctrl_->strong_count == 0) {
            // destroy managed object
            delete ctrl_->ptr;
            ctrl_->ptr = nullptr;

            // if no weak refs either, delete control block
            if (ctrl_->weak_count == 0) {
                delete ctrl_;
            }
        }

        ctrl_ = nullptr;
        ptr_  = nullptr;
    }

    ControlBlock<T>* ctrl_{nullptr};
    T*               ptr_{nullptr};
};

template <typename T>
class WeakPtr {
public:
    constexpr WeakPtr() noexcept = default;

    WeakPtr(const SharedPtr<T>& sp) noexcept
        : ctrl_(sp.ctrl_) {
        inc_weak();
    }

    WeakPtr(const WeakPtr& other) noexcept
        : ctrl_(other.ctrl_) {
        inc_weak();
    }

    WeakPtr& operator=(const WeakPtr& other) noexcept {
        if (this == &other) return *this;
        release();
        ctrl_ = other.ctrl_;
        inc_weak();
        return *this;
    }

    WeakPtr(WeakPtr&& other) noexcept
        : ctrl_(other.ctrl_) {
        other.ctrl_ = nullptr;
    }

    WeakPtr& operator=(WeakPtr&& other) noexcept {
        if (this == &other) return *this;
        release();
        ctrl_       = other.ctrl_;
        other.ctrl_ = nullptr;
        return *this;
    }

    ~WeakPtr() {
        release();
    }

    void reset() noexcept {
        release();
    }

    [[nodiscard]] bool expired() const noexcept {
        return !ctrl_ || ctrl_->strong_count == 0;
    }

    [[nodiscard]] std::size_t use_count() const noexcept {
        return ctrl_ ? ctrl_->strong_count : 0;
    }

    // Try to obtain a SharedPtr; returns empty if expired
    [[nodiscard]] SharedPtr<T> lock() const noexcept {
        if (expired()) {
            return SharedPtr<T>{};
        }
        return SharedPtr<T>(ctrl_);
    }

private:
    void inc_weak() noexcept {
        if (ctrl_) {
            ++ctrl_->weak_count;
        }
    }

    void release() noexcept {
        if (!ctrl_) return;

        if (--ctrl_->weak_count == 0 && ctrl_->strong_count == 0) {
            delete ctrl_;
        }
        ctrl_ = nullptr;
    }

    ControlBlock<T>* ctrl_{nullptr};
};

// make_shared equivalent
template <typename T, typename... Args>
[[nodiscard]] SharedPtr<T> make_shared(Args&&... args) {
    T* raw = new T(std::forward<Args>(args)...);
    return SharedPtr<T>(raw);
}

} // namespace smart_pointers

