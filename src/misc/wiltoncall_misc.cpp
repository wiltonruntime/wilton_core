/* 
 * File:   wiltoncall_misc.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 12:34 PM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace misc {

namespace { //anonymous

namespace ss = staticlib::serialization;

} // namespace

std::string tcp_wait_for_connection(const std::string& data) {
    // json parse
    ss::json_value json = ss::load_json_from_string(data);
    int64_t timeout = -1;
    auto rip = std::ref(common::empty_string());
    int64_t port = -1;
    for (const ss::json_field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("ipAddress" == name) {
            rip = common::get_json_string(fi);
        } else if ("tcpPort" == name) {
            port = common::get_json_int64(fi);
        } else if ("timeoutMillis" == name) {
            timeout = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == timeout) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'timeoutMillis' not specified"));
    if (rip.get().empty()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'ipAddress' not specified"));
    if (-1 == port) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'tcpPort' not specified"));
    const std::string& ip = rip.get();
    // call wilton
    char* err = wilton_tcp_wait_for_connection(ip.c_str(), ip.size(),
            static_cast<int> (port), static_cast<int> (timeout));
    if (nullptr != err) {
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

} // namespace
}
