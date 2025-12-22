#include <iostream>
#include "lru_cache/lru_cache.hpp"

int main()
{
    lru::LRUCache<int, int> cache(3); // capacity = 3

    cache.put(1, 10);
    cache.put(2, 20);
    cache.put(3, 30);

    auto print_get = [&](int key) {
        auto v = cache.get(key);
        std::cout << "Get " << key << ": "
                  << (v ? std::to_string(*v) : std::string{"<miss>"})
                  << '\n';
    };

    print_get(1); // 10
    print_get(2); // 20

    cache.put(4, 40); // evicts key 3 (LRU)

    print_get(3); // <miss>
    print_get(4); // 40

    cache.put(5, 50); // should evict key 1 (LRU after accesses)

    print_get(1); // <miss>
    print_get(2); // 20
    print_get(5); // 50

    return 0;
}

