/* 
 * File:   ClientRequestConfig.hpp
 * Author: alex
 *
 * Created on June 13, 2016, 4:24 PM
 */

#ifndef WILTON_CLIENT_CLIENTREQUESTCONFIG_HPP
#define	WILTON_CLIENT_CLIENTREQUESTCONFIG_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "staticlib/httpclient.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"

namespace wilton {
namespace client {

class ClientRequestConfig {
public:
    staticlib::httpclient::HttpRequestOptions options;

    ClientRequestConfig(const ClientRequestConfig&) = delete;

    ClientRequestConfig& operator=(const ClientRequestConfig&) = delete;

    ClientRequestConfig(ClientRequestConfig&& other) :
    options(std::move(other.options)) { }

    ClientRequestConfig& operator=(ClientRequestConfig&& other) {
        this->options = std::move(other.options);
        return *this;
    }

    ClientRequestConfig() { }

    ClientRequestConfig(const staticlib::serialization::JsonValue& json) {
        namespace ss = staticlib::serialization;
        for (const ss::JsonField& fi : json.get_object()) {
            auto& name = fi.get_name();
            if ("headers" == name) {
                for (const ss::JsonField& hf : common::get_json_object(fi, "headers")) {
                    std::string val = common::get_json_string(hf, std::string("headers.") + hf.get_name());
                    options.headers.emplace_back(hf.get_name(), std::move(val));
                }
            } else if ("method" == name) {
                options.method = common::get_json_string(fi, "method");
            } else if ("abortOnResponseError" == name) {
                options.abort_on_response_error = common::get_json_bool(fi, "abortOnResponseError");
            } else if ("readTimeoutMillis" == name) {
                options.read_timeout_millis = common::get_json_uint32(fi, "readTimeoutMillis");
            } else if ("fdsetTimeoutMillis" == name) {
                options.fdset_timeout_millis = common::get_json_uint32(fi, "fdsetTimeoutMillis");
            } else if ("forceHttp10" == name) {
                options.force_http_10 = common::get_json_bool(fi, "forceHttp10");
            } else if ("noprogress" == name) {
                options.noprogress = common::get_json_bool(fi, "noprogress");
            } else if ("nosignal" == name) {
                options.nosignal = common::get_json_bool(fi, "nosignal");
            } else if ("failonerror" == name) {
                options.failonerror = common::get_json_bool(fi, "failonerror");
            } else if ("pathAsIs" == name) {
                options.path_as_is = common::get_json_bool(fi, "pathAsIs");
            } else if ("tcpNodelay" == name) {
                options.tcp_nodelay = common::get_json_bool(fi, "tcpNodelay");
            } else if ("tcpKeepalive" == name) {
                options.tcp_keepalive = common::get_json_bool(fi, "tcpKeepalive");
            } else if ("tcpKeepidleSecs" == name) {
                options.tcp_keepidle_secs = common::get_json_uint32(fi, "tcpKeepidleSecs");
            } else if ("tcpKeepintvlSecs" == name) {
                options.tcp_keepintvl_secs = common::get_json_uint32(fi, "tcpKeepintvlSecs");
            } else if ("connecttimeoutMillis" == name) {
                options.connecttimeout_millis = common::get_json_uint32(fi, "connecttimeoutMillis");
            } else if ("buffersizeBytes" == name) {
                options.buffersize_bytes = common::get_json_uint32(fi, "buffersizeBytes");
            } else if ("acceptEncoding" == name) {
                options.accept_encoding = common::get_json_string(fi, "acceptEncoding");
            } else if ("followlocation" == name) {
                options.followlocation = common::get_json_bool(fi, "followlocation");
            } else if ("maxredirs" == name) {
                options.maxredirs = common::get_json_uint32(fi, "maxredirs");
            } else if ("useragent" == name) {
                options.useragent = common::get_json_string(fi, "useragent");
            } else if ("maxSentSpeedLargeBytesPerSecond" == name) {
                options.max_sent_speed_large_bytes_per_second = common::get_json_uint32(fi, "maxSentSpeedLargeBytesPerSecond");
            } else if ("maxRecvSpeedLargeBytesPerSecond" == name) {
                options.max_recv_speed_large_bytes_per_second = common::get_json_uint32(fi, "maxRecvSpeedLargeBytesPerSecond");
            } else if ("sslcertFilename" == name) {
                options.sslcert_filename = common::get_json_string(fi, "sslcertFilename");
            } else if ("sslcertype" == name) {
                options.sslcertype = common::get_json_string(fi, "sslcertype");
            } else if ("sslkeyFilename" == name) {
                options.sslkey_filename = common::get_json_string(fi, "sslkeyFilename");
            } else if ("sslKeyType" == name) {
                options.ssl_key_type = common::get_json_string(fi, "sslKeyType");
            } else if ("sslKeypasswd" == name) {
                options.ssl_keypasswd = common::get_json_string(fi, "sslKeypasswd");
            } else if ("requireTls" == name) {
                options.require_tls = common::get_json_bool(fi, "requireTls");
            } else if ("sslVerifyhost" == name) {
                options.ssl_verifyhost = common::get_json_bool(fi, "sslVerifyhost");
            } else if ("sslVerifypeer" == name) {
                options.ssl_verifypeer = common::get_json_bool(fi, "sslVerifypeer");
            } else if ("sslVerifystatus" == name) {
                options.ssl_verifystatus = common::get_json_bool(fi, "sslVerifystatus");
            } else if ("cainfoFilename" == name) {
                options.cainfo_filename = common::get_json_string(fi, "cainfoFilename");
            } else if ("crlfileFilename" == name) {
                options.crlfile_filename = common::get_json_string(fi, "crlfileFilename");
            } else if ("sslCipherList" == name) {
                options.ssl_cipher_list = common::get_json_string(fi, "sslCipherList");
            } else {
                throw common::WiltonInternalException(TRACEMSG("Unknown 'ClientRequest' field: [" + name + "]"));
            }
        }
    }

    staticlib::serialization::JsonValue to_json() const {
        namespace sr = staticlib::ranges;
        namespace ss = staticlib::serialization;
        auto ha = sr::transform(sr::refwrap(options.headers), [](const std::pair<std::string, std::string>& el) {
            return ss::JsonField{el.first, el.second};
        });
        std::vector<ss::JsonField> hfields = sr::emplace_to_vector(std::move(ha));
        return {
            {"headers", std::move(hfields)},
            {"method", options.method},
            {"abortOnResponseError", options.abort_on_response_error},
            {"readTimeoutMillis", options.read_timeout_millis},
            {"fdsetTimeoutMillis", options.fdset_timeout_millis},
            {"forceHttp10", options.force_http_10},
            {"noprogress", options.noprogress},
            {"nosignal", options.nosignal},
            {"failonerror", options.failonerror},
            {"pathAsIs", options.path_as_is},
            {"tcpNodelay", options.tcp_nodelay},
            {"tcpKeepalive", options.tcp_keepalive},
            {"tcpKeepidleSecs", options.tcp_keepidle_secs},
            {"tcpKeepintvlSecs", options.tcp_keepintvl_secs},
            {"connecttimeoutMillis", options.connecttimeout_millis},
            {"buffersizeBytes", options.buffersize_bytes},
            {"acceptEncoding", options.accept_encoding},
            {"followlocation", options.followlocation},
            {"maxredirs", options.maxredirs},
            {"useragent", options.useragent},
            {"maxSentSpeedLargeBytesPerSecond", options.max_sent_speed_large_bytes_per_second},
            {"maxRecvSpeedLargeBytesPerSecond", options.max_recv_speed_large_bytes_per_second},
            {"sslcertFilename", options.sslcert_filename},
            {"sslcertype", options.sslcertype},
            {"sslkeyFilename", options.sslkey_filename},
            {"sslKeyType", options.ssl_key_type},
            {"sslKeypasswd", options.ssl_keypasswd},
            {"requireTls", options.require_tls},
            {"sslVerifyhost", options.ssl_verifyhost},
            {"sslVerifypeer", options.ssl_verifypeer},
            {"sslVerifystatus", options.ssl_verifystatus},
            {"cainfoFilename", options.cainfo_filename},
            {"crlfileFilename", options.crlfile_filename},
            {"sslCipherList", options.ssl_cipher_list}
        };
    }

};

} // namespace
} 

#endif	/* WILTON_CLIENT_CLIENTREQUESTCONFIG_HPP */

