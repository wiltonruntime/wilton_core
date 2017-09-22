/* 
 * File:   document_root.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:27 PM
 */

#ifndef WILTON_SERVERCONF_DOCUMENT_ROOT_HPP
#define WILTON_SERVERCONF_DOCUMENT_ROOT_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/json.hpp"

#include "wilton/support/exception.hpp"

#include "serverconf/mime_type.hpp"

namespace wilton {
namespace serverconf {

class document_root {
public:
    std::string resource = "";
    std::string dirPath = "";
    std::string zipPath = "";
    std::string zipInnerPrefix = "";
    uint32_t cacheMaxAgeSeconds = 604800;
    std::vector<mime_type> mimeTypes = default_mimes();
    
    document_root(const document_root&) = delete;
    
    document_root& operator=(const document_root&) = delete;

    document_root(document_root&& other) :
    resource(std::move(other.resource)),
    dirPath(std::move(other.dirPath)),
    zipPath(std::move(other.zipPath)),
    zipInnerPrefix(std::move(other.zipInnerPrefix)),
    cacheMaxAgeSeconds(other.cacheMaxAgeSeconds),
    mimeTypes(std::move(other.mimeTypes)) { }

    document_root& operator=(document_root&& other) {
        this->resource = std::move(other.resource);
        this->dirPath = std::move(other.dirPath);
        this->zipPath = std::move(other.zipPath);
        this->zipInnerPrefix = std::move(other.zipInnerPrefix);
        this->cacheMaxAgeSeconds = other.cacheMaxAgeSeconds;
        this->mimeTypes = std::move(other.mimeTypes);
        return *this;
    }
    
    document_root() { }
    
    document_root(const std::string& resource, const std::string& dirPath, 
            const std::string& zipPath, const std::string& zipInnerPrefix,
            uint32_t cacheMaxAgeSeconds, const std::vector<mime_type>& mimeTypes) :
    resource(resource.data(), resource.length()), 
    dirPath(dirPath.data(), dirPath.length()), 
    zipPath(zipPath.data(), zipPath.length()), 
    zipInnerPrefix(zipInnerPrefix.data(), zipInnerPrefix.length()), 
    cacheMaxAgeSeconds(cacheMaxAgeSeconds), 
    mimeTypes(mimes_copy(mimeTypes)) { }
    
    document_root(const sl::json::value& json) {
        for (const sl::json::field& fi : json.as_object()) {
            auto& name = fi.name();
            if ("resource" == name) {
                this->resource = fi.as_string_nonempty_or_throw(name);
            } else if ("dirPath" == name) {
                this->dirPath = fi.as_string_nonempty_or_throw(name);
            } else if ("zipPath" == name) {
                this->zipPath = fi.as_string_nonempty_or_throw(name);
            } else if ("zipInnerPrefix" == name) {
                this->zipInnerPrefix = fi.as_string_nonempty_or_throw(name);
            } else if ("cacheMaxAgeSeconds" == name) {
                this->cacheMaxAgeSeconds = fi.as_uint32_or_throw(name);
            } else if ("mimeTypes" == name) {
                for (const sl::json::value& ap : fi.as_array_or_throw(name)) {
                    auto ja = serverconf::mime_type(ap);
                    mimeTypes.emplace_back(std::move(ja));
                }
            } else {
                throw support::exception(TRACEMSG("Unknown 'documentRoot' field: [" + name + "]"));
            }
        }
        if (0 == resource.length()) throw support::exception(TRACEMSG(
                    "Invalid 'documentRoot.resource' field: []"));
        if (0 == dirPath.length() && 0 == zipPath.length()) throw support::exception(TRACEMSG(
                    "Invalid 'documentRoot.dirPath' and 'documentRoot.zipPath' fields: [], []"));
    }
       
    sl::json::value to_json() const {
        namespace sr = staticlib::ranges;
        return {
            {"resource", resource},
            {"dirPath", dirPath},
            {"zipPath", zipPath},
            {"zipInnerPrefix", zipInnerPrefix},
            {"cacheMaxAgeSeconds", cacheMaxAgeSeconds},
            {"mimeTypes", [this] {
                auto ra = sl::ranges::transform(sl::ranges::refwrap(mimeTypes), [](const mime_type& el) {
                    return el.to_json();
                });
                return ra.to_vector();
            }()}
        };
    }
    
    bool is_empty() {
        return 0 == resource.length();
    }
    
    document_root clone() const {
        return document_root(resource, dirPath, zipPath, zipInnerPrefix, cacheMaxAgeSeconds, mimeTypes);
    }
    
private:
    static std::vector<mime_type> mimes_copy(const std::vector<mime_type>& vec) {
        namespace sr = staticlib::ranges;
        auto copied = sl::ranges::transform(sl::ranges::refwrap(vec), [](const mime_type & el) {
            return el.clone();
        });
        return sl::ranges::emplace_to_vector(std::move(copied));
    }

    static std::vector<mime_type> default_mimes() {
        std::vector<mime_type> res{};
        res.emplace_back("txt", "text/plain");
        res.emplace_back("js", "text/javascript");
        res.emplace_back("css", "text/css");
        res.emplace_back("html", "text/html");
        res.emplace_back("png", "image/png");
        res.emplace_back("jpg", "image/jpeg");
        res.emplace_back("svg", "image/svg+xml");
        return res;
    }

};

} // namespace
}

#endif /* WILTON_SERVERCONF_DOCUMENT_ROOT_HPP */

