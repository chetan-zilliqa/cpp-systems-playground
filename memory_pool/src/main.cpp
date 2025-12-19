#include <iostream>
#include "common/logging.hpp"
#include "memory_pool/fixed_block_memory_pool.hpp"

/*
memory pool:
improves spatial locality as linkedlist nodes are allocated continguously
rather than random address which reduces cache misses.
makes pointer-based structures competitive
*/
using memory_pool::FixedBlockMemoryPool;

struct Node {
    int value;
    Node* next;
};

int main() {
    common::set_log_level(common::LogLevel::Debug);

    LOG_INFO("Starting FixedBlockMemoryPool demo");

    constexpr std::size_t capacity = 8;
    FixedBlockMemoryPool pool(sizeof(Node), capacity);

    std::cout << "========================================\n";
    std::cout << "  FixedBlockMemoryPool demo\n";
    std::cout << "========================================\n";
    std::cout << "Block size: " << pool.block_size() << " bytes\n";
    std::cout << "Capacity  : " << pool.capacity() << " blocks\n";

    Node* head = nullptr;

    // If we add more than 8 nodes, the program would crash as we allocated
    //space for 8 nodes
    for (int i = 0; i < 8; ++i) {
        LOG_DEBUG("Allocating node " + std::to_string(i));
        void* raw = pool.allocate();
        auto* node = new (raw) Node{i, head};
        head = node;
    }

    std::cout << "List contents from head:\n";
    for (Node* cur = head; cur; cur = cur->next) {
        std::cout << "  Node value = " << cur->value
                  << " at " << static_cast<void*>(cur) << "\n";
    }

    while (head) {
        LOG_DEBUG("Deallocating node with value " + std::to_string(head->value));
        Node* next = head->next;
        head->~Node();
        pool.deallocate(head);
        head = next;
    }

    LOG_INFO("All nodes destroyed and deallocated back to the pool");

    std::cout << "All nodes destroyed and deallocated back to the pool.\n";
    return 0;
}

