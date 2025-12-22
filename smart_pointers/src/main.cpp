#include <iostream>
#include <string>

#include "smart_pointers/unique_ptr.hpp"
#include "smart_pointers/shared_ptr.hpp"

using namespace smart_pointers;

struct Foo {
    int value;
    explicit Foo(int v) : value(v) {
        std::cout << "Foo(" << value << ") constructed\n";
    }
    ~Foo() {
        std::cout << "Foo(" << value << ") destructed\n";
    }
};

static void demo_unique_ptr() {
    std::cout << "=== UniquePtr demo ===\n";

    auto up = make_unique<Foo>(42);
    if (up) {
        std::cout << "value = " << up->value << "\n";
    }

    // transfer ownership
    UniquePtr<Foo> up2 = std::move(up);
    std::cout << "up is " << (up ? "not null" : "null") << "\n";
    std::cout << "up2->value = " << up2->value << "\n";

    up2.reset(); // calls deleter
    std::cout << "UniquePtr reset() done\n\n";
}

static void demo_shared_ptr() {
    std::cout << "=== SharedPtr / WeakPtr demo ===\n";

    auto sp1 = make_shared<Foo>(100);
    std::cout << "use_count after sp1: " << sp1.use_count() << "\n";

    {
        SharedPtr<Foo> sp2 = sp1;
        std::cout << "use_count after sp2 copy: " << sp1.use_count() << "\n";

        WeakPtr<Foo> wp(sp1);
        std::cout << "weak use_count: " << wp.use_count() << "\n";

        if (auto locked = wp.lock()) {
            std::cout << "locked->value = " << locked->value << "\n";
        }

        // sp2 goes out of scope here
    }

    std::cout << "use_count after sp2 destroyed: " << sp1.use_count() << "\n";

    sp1.reset(); // destroys Foo when strong_count hits 0

    WeakPtr<Foo> wp2; // empty weak
    {
        auto sp3 = make_shared<Foo>(200);
        wp2 = WeakPtr<Foo>(sp3);
        std::cout << "wp2.use_count = " << wp2.use_count() << "\n";
    } // sp3 destroyed, Foo(200) destroyed

    std::cout << "wp2.expired() = " << (wp2.expired() ? "true" : "false") << "\n\n";
}

int main() {
    demo_unique_ptr();
    demo_shared_ptr();
    return 0;
}

