/* 
 * File:   handle_registry.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 12:32 PM
 */

#ifndef WILTON_SUPPORT_HANDLE_REGISTRY_HPP
#define	WILTON_SUPPORT_HANDLE_REGISTRY_HPP

#include <cstdint>
#include <mutex>
#include <unordered_set>

namespace wilton {
namespace support {

template<typename T>
class handle_registry {
    std::unordered_set<T*> registry;
    std::mutex mutex;

public:

    int64_t put(T* ptr) {
        std::lock_guard<std::mutex> lock{mutex};
        auto pair = registry.insert(ptr);
        return pair.second ? reinterpret_cast<int64_t> (ptr) : 0;
    }

    T* remove(int64_t handle) {
        std::lock_guard<std::mutex> lock{mutex};
        T* ptr = reinterpret_cast<T*> (handle);
        auto erased = registry.erase(ptr);
        return 1 == erased ? ptr : nullptr;
    }

    T* peek(int64_t handle) {
        std::lock_guard<std::mutex> lock{mutex};
        T* ptr = reinterpret_cast<T*> (handle);
        auto exists = registry.count(ptr);
        return 1 == exists ? ptr : nullptr;
    }
};

} // namespace
}

#endif	/* WILTON_SUPPORT_HANDLE_REGISTRY_HPP */

