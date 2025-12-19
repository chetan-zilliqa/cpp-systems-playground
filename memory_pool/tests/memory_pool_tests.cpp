#include <cassert>
#include <iostream>
#include "memory_pool/fixed_block_memory_pool.hpp"

using memory_pool::FixedBlockMemoryPool;

struct Node {
    int value;
    Node* next;
};

int main() {
    // Basic construction
    FixedBlockMemoryPool pool(sizeof(Node), 4);
    assert(pool.block_size() >= sizeof(Node));
    assert(pool.capacity() == 4);

    // Allocate up to capacity
    void* p1 = pool.allocate();
    void* p2 = pool.allocate();
    void* p3 = pool.allocate();
    void* p4 = pool.allocate();

    assert(p1 != nullptr);
    assert(p2 != nullptr);
    assert(p3 != nullptr);
    assert(p4 != nullptr);

    // Next allocate should throw std::bad_alloc
    bool threw = false;
    try {
        (void)pool.allocate();
    } catch (const std::bad_alloc&) {
        threw = true;
    }
    assert(threw && "allocate() should throw when pool exhausted");

    // Deallocate one block, then allocate again and expect reuse
    pool.deallocate(p4);
    void* p5 = pool.allocate();
    assert(p5 == p4 && "pool should reuse last freed block");

    // Use placement new / destroy with Node
    auto* n1 = new (p1) Node{42, nullptr};
    assert(n1->value == 42);
    n1->~Node();
    pool.deallocate(n1);

    return 0;
}
