/* 
 * File:   client_response.hpp
 * Author: alex
 *
 * Created on June 13, 2016, 4:26 PM
 */

#ifndef WILTON_CLIENT_CLIENT_RESPONSE_HPP
#define WILTON_CLIENT_CLIENT_RESPONSE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/http.hpp"
#include "staticlib/json.hpp"
#include "staticlib/ranges.hpp"

#include "wilton/support/exception.hpp"

namespace wilton {
namespace client {

class client_response {
public:
    static sl::json::value to_json(std::string&& data_hex, const sl::http::resource& resource,
            const sl::http::resource_info& info) {
        auto ha = sl::ranges::transform(resource.get_headers(), [](const std::pair<std::string, std::string>& el) {
            return sl::json::field{el.first, el.second};
        });
        std::vector<sl::json::field> hfields = sl::ranges::emplace_to_vector(std::move(ha));
        return {
            {"connectionSuccess", resource.connection_successful()},
            {"dataHex", std::move(data_hex)},
            {"headers", std::move(hfields)},
            {"effectiveUrl", info.effective_url},
            {"responseCode", static_cast<int64_t> (resource.get_status_code())},
            {"totalTimeSecs", info.total_time_secs},
            {"namelookupTimeSecs", info.namelookup_time_secs},
            {"connectTimeSecs", info.connect_time_secs},
            {"appconnectTimeSecs", info.appconnect_time_secs},
            {"pretransferTimeSecs", info.pretransfer_time_secs},
            {"starttransferTimeSecs", info.starttransfer_time_secs},
            {"redirectTimeSecs", info.redirect_time_secs},
            {"redirectCount", static_cast<int64_t> (info.redirect_count)},
            {"speedDownloadBytesSecs", info.speed_download_bytes_secs},
            {"speedUploadBytesSecs", info.speed_upload_bytes_secs},
            {"headerSizeBytes", static_cast<int64_t> (info.header_size_bytes)},
            {"requestSizeBytes", static_cast<int64_t> (info.request_size_bytes)},
            {"sslVerifyresult", static_cast<int64_t> (info.ssl_verifyresult)},
            {"osErrno", static_cast<int64_t> (info.os_errno)},
            {"numConnects", static_cast<int64_t> (info.num_connects)},
            {"primaryIp", info.primary_ip},
            {"primaryPort", static_cast<int64_t> (info.primary_port)}
        };
    }
};
    
} // namespace
}

#endif /* WILTON_CLIENT_CLIENT_RESPONSE_HPP */

