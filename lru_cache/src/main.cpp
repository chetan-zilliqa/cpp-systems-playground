#include <iostream>
#include <string>

#include "lru_cache/lru_cache.hpp"

int main()
{
    using Cache = lru::LRUCache<int, std::string>;

    Cache cache(3);

    std::cout << "=== LRUCache demo ===\n";

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");

    auto print_get = [&](int key) {
        auto v = cache.get(key);
        std::cout << "get(" << key << ") -> "
                  << (v ? *v : std::string{"<miss>"}) << '\n';
    };

    print_get(1); // hit, 1 becomes MRU
    print_get(2); // hit, 2 becomes MRU

    std::cout << "Current size: " << cache.size()
              << " / capacity: " << cache.capacity() << "\n";

    cache.put(4, "four"); // should evict LRU (key 3)

    print_get(3); // miss
    print_get(4); // hit

    cache.put(5, "five"); // evicts LRU among {1,2,4}

    print_get(1); // might be evicted depending on access order
    print_get(2);
    print_get(4);
    print_get(5);

    return 0;
}

