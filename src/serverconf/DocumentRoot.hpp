/* 
 * File:   DocumentRoot.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:27 PM
 */

#ifndef WILTON_SERVERCONF_DOCUMENTROOT_HPP
#define	WILTON_SERVERCONF_DOCUMENTROOT_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"
#include "serverconf/MimeType.hpp"

namespace wilton {
namespace serverconf {

class DocumentRoot {
public:
    std::string resource = "";
    std::string dirPath = "";
    std::string zipPath = "";
    std::string zipInnerPrefix = "";
    uint32_t cacheMaxAgeSeconds = 604800;
    std::vector<MimeType> mimeTypes = default_mimes();
    
    DocumentRoot(const DocumentRoot&) = delete;
    
    DocumentRoot& operator=(const DocumentRoot&) = delete;

    DocumentRoot(DocumentRoot&& other) :
    resource(std::move(other.resource)),
    dirPath(std::move(other.dirPath)),
    zipPath(std::move(other.zipPath)),
    zipInnerPrefix(std::move(other.zipInnerPrefix)),
    cacheMaxAgeSeconds(other.cacheMaxAgeSeconds),
    mimeTypes(std::move(other.mimeTypes)) { }

    DocumentRoot& operator=(DocumentRoot&& other) {
        this->resource = std::move(other.resource);
        this->dirPath = std::move(other.dirPath);
        this->zipPath = std::move(other.zipPath);
        this->zipInnerPrefix = std::move(other.zipInnerPrefix);
        this->cacheMaxAgeSeconds = other.cacheMaxAgeSeconds;
        this->mimeTypes = std::move(other.mimeTypes);
        return *this;
    }
    
    DocumentRoot() { }
    
    DocumentRoot(const std::string& resource, const std::string& dirPath, 
            const std::string& zipPath, const std::string& zipInnerPrefix,
            uint32_t cacheMaxAgeSeconds, const std::vector<MimeType>& mimeTypes) :
    resource(resource.data(), resource.length()), 
    dirPath(dirPath.data(), dirPath.length()), 
    zipPath(zipPath.data(), zipPath.length()), 
    zipInnerPrefix(zipInnerPrefix.data(), zipInnerPrefix.length()), 
    cacheMaxAgeSeconds(cacheMaxAgeSeconds), 
    mimeTypes(mimes_copy(mimeTypes)) { }
    
    DocumentRoot(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.as_object()) {
            auto& name = fi.name();
            if ("resource" == name) {
                this->resource = common::get_json_string(fi, "documentRoot.resource");
            } else if ("dirPath" == name) {
                this->dirPath = common::get_json_string(fi, "documentRoot.dirPath");
            } else if ("zipPath" == name) {
                this->zipPath = common::get_json_string(fi, "documentRoot.zipPath");
            } else if ("zipInnerPrefix" == name) {
                this->zipInnerPrefix = common::get_json_string(fi, "documentRoot.zipInnerPrefix");
            } else if ("cacheMaxAgeSeconds" == name) {
                this->cacheMaxAgeSeconds = common::get_json_uint32(fi, "documentRoot.cacheMaxAgeSeconds");
            } else if ("mimeTypes" == name) {
                for (const ss::JsonValue& ap : common::get_json_array(fi, "documentRoot.mimeTypes")) {
                    auto ja = serverconf::MimeType(ap);
                    mimeTypes.emplace_back(std::move(ja));
                }
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'documentRoot' field: [" + name + "]"));
            }
        }
        if (0 == resource.length()) throw common::WiltonInternalException(TRACEMSG(
                    "Invalid 'documentRoot.resource' field: []"));
        if (0 == dirPath.length() && 0 == zipPath.length()) throw common::WiltonInternalException(TRACEMSG(
                    "Invalid 'documentRoot.dirPath' and 'documentRoot.zipPath' fields: [], []"));
    }
       
    staticlib::serialization::JsonValue to_json() const {
        namespace sr = staticlib::ranges;
        auto mimes = sr::transform(sr::refwrap(mimeTypes), [](const MimeType& el) {
            return el.to_json();
        });
        return {
            {"resource", resource},
            {"dirPath", dirPath},
            {"zipPath", zipPath},
            {"zipInnerPrefix", zipInnerPrefix},
            {"cacheMaxAgeSeconds", cacheMaxAgeSeconds},
            {"mimeTypes", mimes}
        };
    }
    
    bool is_empty() {
        return 0 == resource.length();
    }
    
    DocumentRoot clone() const {
        return DocumentRoot(resource, dirPath, zipPath, zipInnerPrefix, cacheMaxAgeSeconds, mimeTypes);
    }
    
private:
    static std::vector<MimeType> mimes_copy(const std::vector<MimeType>& vec) {
        namespace sr = staticlib::ranges;
        auto copied = sr::transform(sr::refwrap(vec), [](const MimeType & el) {
            return el.clone();
        });
        return sr::emplace_to_vector(std::move(copied));
    }

    static std::vector<MimeType> default_mimes() {
        std::vector<MimeType> res{};
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

#endif	/* WILTON_SERVERCONF_DOCUMENTROOT_HPP */

