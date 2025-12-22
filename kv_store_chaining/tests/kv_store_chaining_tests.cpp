#include <cassert>
#include <string>
#include "kv_store_chaining/kv_store_chaining.hpp"

using kv_store::InMemorykvstore_chaining;

int main() {
    InMemorykvstore_chaining store(8, 16);

    // Initially empty
    assert(store.size() == 0);
    assert(!store.contains("a"));
    assert(!store.get("a").has_value());

    // Put and get
    store.put("a", "1");
    store.put("b", "2");

    assert(store.size() == 2);
    assert(store.contains("a"));
    assert(store.contains("b"));
    assert(store.get("a").value() == "1");
    assert(store.get("b").value() == "2");

    // Overwrite existing key
    store.put("a", "42");
    assert(store.size() == 2);
    assert(store.get("a").value() == "42");

    // Erase existing key
    bool erased_a = store.erase("a");
    assert(erased_a);
    assert(store.size() == 1);
    assert(!store.contains("a"));
    assert(!store.get("a").has_value());

    // Erase non-existing key
    bool erased_missing = store.erase("missing");
    assert(!erased_missing);

    // Put enough items to stress pool capacity a bit
    for (int i = 0; i < 10; ++i) {
        store.put("k" + std::to_string(i), "v" + std::to_string(i));
    }

    // We don't know exact size because some keys may collide / overwrite,
    // but basic API should still behave correctly for a sample key.
    assert(store.contains("k0"));
    assert(store.get("k0").has_value());
    assert(store.get("k0").value() == "v0");

    return 0;
}
