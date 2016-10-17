/* 
 * File:   ClientResponse.hpp
 * Author: alex
 *
 * Created on June 13, 2016, 4:26 PM
 */

#ifndef WILTON_CLIENT_CLIENTRESPONSE_HPP
#define	WILTON_CLIENT_CLIENTRESPONSE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/httpclient.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace client {

class ClientResponse {
public:
    static staticlib::serialization::JsonValue to_json(std::string&& data,
            const staticlib::httpclient::HttpResourceInfo& info) {
        namespace sr = staticlib::ranges;
        namespace ss = staticlib::serialization;
        auto ha = sr::transform(sr::refwrap(info.get_headers()), [](const std::pair<std::string, std::string>& el) {
            return ss::JsonField{el.first, el.second};
        });
        std::vector<ss::JsonField> hfields = sr::emplace_to_vector(std::move(ha));
        return {
            {"connectionSuccess", info.connection_success()},
            {"data", std::move(data)},
            {"headers", std::move(hfields)},
            {"effectiveUrl", info.effective_url},
            {"responseCode", static_cast<int64_t> (info.response_code)},
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

#endif	/* WILTON_CLIENT_CLIENTRESPONSE_HPP */

