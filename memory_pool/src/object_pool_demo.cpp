#include <iostream>
#include <string>
#include "memory_pool/object_pool.hpp"   // header-only pool in memory_pool/include/

using memory_pool::ObjectPool;
struct Packet {

    int id;
    std::string payload;

    Packet(int i = 0, std::string data = {})
        : id(i), payload(std::move(data))
    {
        std::cout << "[ctor] Packet " << id
                  << " created (payload size: " << payload.size() << ")\n";
    }

    ~Packet() {
        std::cout << "[dtor] Packet " << id << " destroyed\n";
    }
};

static void demo_basic_usage()
{
    std::cout << "\n=== ObjectPool basic usage demo ===\n";

    constexpr std::size_t capacity = 3;
    ObjectPool<Packet, capacity> pool;

    std::cout << "Pool capacity: " << pool.capacity() << "\n";
    std::cout << "Initial free slots: " << pool.free_slots() << "\n";

    {
        std::cout << "Allocating p1, p2...\n";
        auto p1 = pool.make_unique(42, "hello from packet 42");
        auto p2 = pool.make_unique(99, "packet 99 with a longer payload");

        std::cout << "After 2 allocs: free slots = " << pool.free_slots() << "\n";
        std::cout << "p1 -> id=" << p1->id << ", payload='" << p1->payload << "'\n";
        std::cout << "p2 -> id=" << p2->id << ", payload='" << p2->payload << "'\n";

        std::cout << "Allocating p3...\n";
        auto p3 = pool.make_unique(7, "third packet");

        std::cout << "After 3 allocs: free slots = " << pool.free_slots() << "\n";
        std::cout << "p3 -> id=" << p3->id << ", payload='" << p3->payload << "'\n";

        std::cout << "Leaving inner scope, unique_ptr destructors will return "
                     "all packets to the pool\n";
    }

    std::cout << "After scope: free slots = " << pool.free_slots() << "\n";

    std::cout << "Allocating p4 (reuse)...\n";
    auto p4 = pool.make_unique(123, "reused slot");

    std::cout << "p4 -> id=" << p4->id << ", payload='" << p4->payload << "'\n";
    std::cout << "Free slots after p4 = " << pool.free_slots() << "\n";
}

static void demo_exhaustion()
{
    std::cout << "\n=== ObjectPool exhaustion demo ===\n";

    ObjectPool<Packet, 2> pool;

    auto p1 = pool.make_unique(1, "one");
    auto p2 = pool.make_unique(2, "two");

    std::cout << "Allocated 2 packets, free slots = " << pool.free_slots() << "\n";

    std::cout << "Attempting 3rd allocation...\n";
    try {
        auto p3 = pool.make_unique(3, "three");
        (void)p3;
        std::cout << "ERROR: allocation succeeded but should not\n";
    } catch (const std::bad_alloc&) {
        std::cout << "Caught std::bad_alloc — pool correctly reported full.\n";
    }
}

int main()
{
    demo_basic_usage();
    demo_exhaustion();
    return 0;
}
