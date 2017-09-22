/* 
 * File:   request_metadata.hpp
 * Author: alex
 *
 * Created on May 5, 2016, 7:20 PM
 */

#ifndef WILTON_SERVERCONF_REQUEST_METADATA_HPP
#define WILTON_SERVERCONF_REQUEST_METADATA_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/config.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/json.hpp"

#include "wilton/support/exception.hpp"

#include "serverconf/header.hpp"

namespace wilton {
namespace serverconf {

class request_metadata {
    std::string httpVersion;
    std::string protocol;
    std::string method;
    std::string pathname;
    std::string query;
    std::vector<std::pair<std::string, std::string>> queries;
    std::vector<serverconf::header> headers;

public:
    request_metadata(const request_metadata&) = delete;
    
    request_metadata& operator=(const request_metadata&) = delete;
    
    request_metadata(request_metadata&& other) :
    httpVersion(std::move(other.httpVersion)),
    protocol(std::move(other.protocol)),
    method(std::move(other.method)),
    pathname(std::move(other.pathname)),
    query(std::move(other.query)),
    queries(std::move(other.queries)),
    headers(std::move(other.headers)) { }
    
    request_metadata& operator=(request_metadata&& other) {
        httpVersion = std::move(other.httpVersion);
        protocol = std::move(other.protocol);
        method = std::move(other.method);
        pathname = std::move(other.pathname);
        query = std::move(other.query);
        queries = std::move(other.queries);
        headers = std::move(other.headers);
        return *this;
    }
    
    request_metadata(const std::string& httpVersion, const std::string& protocol, 
            const std::string& method, const std::string& pathname,
            const std::string& query, 
            std::vector<std::pair<std::string, std::string>> queries,
            std::vector<serverconf::header> headers) :
    httpVersion(httpVersion.data(), httpVersion.length()),
    protocol(protocol.data(), protocol.length()),
    method(method.data(), method.length()),
    pathname(pathname.data(), pathname.length()),
    query(query.data(), query.length()),
    queries(std::move(queries)),
    headers(std::move(headers)) { }
        
    sl::json::value to_json() const {
        auto ha = sl::ranges::transform(headers, [](const serverconf::header& el) {
            return el.to_json();
        });
        std::vector<sl::json::field> hfields = sl::ranges::emplace_to_vector(std::move(ha));
        auto qu = sl::ranges::transform(sl::ranges::refwrap(queries), [](const std::pair<std::string, std::string>& pa) {
            return sl::json::field(pa.first, pa.second);
        });
        std::vector<sl::json::field> qfields = sl::ranges::emplace_to_vector(std::move(qu));
        return {
            {"httpVersion", httpVersion},
            {"protocol", protocol},
            {"method", method},
            {"url", reconstructUrl()},
            {"pathname", pathname},
            {"query", query},
            {"queries", std::move(qfields)},
            {"headers", std::move(hfields)}
        };
    }

private:
    std::string reconstructUrl() const {
        if (0 == query.length()) {
            return pathname;
        } else {
            return pathname + "?" + query;
        }
    }
};


} // namespace
}

#endif /* WILTON_SERVERCONF_REQUEST_METADATA_HPP */

