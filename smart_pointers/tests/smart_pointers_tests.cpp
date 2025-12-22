#include "smart_pointers/unique_ptr.hpp"
#include "smart_pointers/shared_ptr.hpp"

#include <cassert>
#include <iostream>

using namespace smart_pointers;

struct TestObj {
    int  value;
    bool* destroyed;

    TestObj(int v, bool* d)
        : value(v), destroyed(d) {}

    ~TestObj() {
        if (destroyed) *destroyed = true;
    }
};

static void test_unique_ptr_basic() {
    bool destroyed = false;

    {
        UniquePtr<TestObj> up(new TestObj(10, &destroyed));
        assert(up);
        assert(up->value == 10);
        assert(!destroyed);

        UniquePtr<TestObj> up2 = std::move(up);
        assert(!up);
        assert(up2);
        assert(up2->value == 10);
    }

    assert(destroyed);
}

static void test_unique_ptr_release_reset() {
    bool destroyed = false;

    UniquePtr<TestObj> up(new TestObj(20, &destroyed));
    auto raw = up.release();
    assert(!up);
    assert(!destroyed); // not destroyed yet

    delete raw; // manual delete
    assert(destroyed);

    // reset on null should be no-op
    up.reset();
}

static void test_shared_ptr_basic() {
    bool destroyed = false;

    {
        auto sp = make_shared<TestObj>(30, &destroyed);
        assert(sp);
        assert(sp.use_count() == 1);
        assert(sp->value == 30);

        {
            SharedPtr<TestObj> sp2 = sp;
            assert(sp.use_count() == 2);
            assert(sp2.use_count() == 2);
        }

        assert(sp.use_count() == 1);
        assert(!destroyed);
    }

    // after sp destroyed
    assert(destroyed);
}

static void test_weak_ptr_lock() {
    bool destroyed = false;

    WeakPtr<TestObj> wp;

    {
        auto sp = make_shared<TestObj>(40, &destroyed);
        wp = WeakPtr<TestObj>(sp);

        assert(!wp.expired());
        assert(wp.use_count() == 1);

        auto locked = wp.lock();
        assert(locked);
        assert(locked->value == 40);
    }

    assert(destroyed);
    assert(wp.expired());
    auto locked2 = wp.lock();
    assert(!locked2);
}

int main() {
    std::cout << "Running smart_pointers tests...\n";

    test_unique_ptr_basic();
    test_unique_ptr_release_reset();
    test_shared_ptr_basic();
    test_weak_ptr_lock();

    std::cout << "All smart_pointers tests passed.\n";
    return 0;
}

