#include <iostream>
#include <string>

#include "hash_map/hash_map.hpp"

int main()
{
    using Map = hash_map::HashMap<std::string, int>;

    Map m;

    std::cout << "=== HashMap demo ===\n";

    m.insert_or_assign("alice", 10);
    m.insert_or_assign("bob",   20);
    m.insert_or_assign("carol", 30);

    // overwrite existing key
    m.insert_or_assign("alice", 42);

    if (auto v = m.get("alice")) {
        std::cout << "alice -> " << *v << "\n";
    }

    std::cout << "contains bob? " << (m.contains("bob") ? "yes" : "no") << "\n";

    m.erase("bob");
    std::cout << "after erase, contains bob? "
              << (m.contains("bob") ? "yes" : "no") << "\n";

    std::cout << "size = " << m.size()
              << ", bucket_count = " << m.bucket_count()
              << ", load_factor = " << m.load_factor()
              << "\n";

    // Reserve more capacity
    m.reserve(100);
    std::cout << "after reserve(100): bucket_count = " << m.bucket_count()
              << ", load_factor = " << m.load_factor()
              << "\n";

    return 0;
}

