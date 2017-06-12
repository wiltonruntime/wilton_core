/* 
 * File:   payload_handle_registry.hpp
 * Author: alex
 *
 * Created on June 12, 2017, 12:38 PM
 */

#ifndef WILTON_SUPPORT_PAYLOAD_HANDLE_REGISTRY_HPP
#define	WILTON_SUPPORT_PAYLOAD_HANDLE_REGISTRY_HPP

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace wilton {
namespace support {

template<typename T, typename P>
class payload_handle_registry {
    std::unordered_map<T*, P> registry;
    std::mutex mutex;

public:
    int64_t put(T* ptr, P&& ctx) {
        std::lock_guard<std::mutex> lock(mutex);
        auto pair = registry.emplace(ptr, std::move(ctx));
        return pair.second ? reinterpret_cast<int64_t> (ptr) : 0;
    }

    std::pair<T*, P> remove(int64_t handle) {
        std::lock_guard<std::mutex> lock(mutex);
        T* ptr = reinterpret_cast<T*> (handle);
        auto it = registry.find(ptr);
        if (registry.end() != it) {
            auto ctx = std::move(it->second);
            registry.erase(ptr);
            return std::make_pair(ptr, std::move(ctx));
        } else {
            return std::make_pair(nullptr, P());
        }
    }
};

} // namespace
}

#endif	/* WILTON_SUPPORT_PAYLOAD_HANDLE_REGISTRY_HPP */

