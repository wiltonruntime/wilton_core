/* 
 * File:   wiltoncall_internal.hpp
 * Author: alex
 *
 * Created on January 10, 2017, 11:42 AM
 */

#ifndef WILTON_CALL_WILTONCALL_INTERNAL_HPP
#define	WILTON_CALL_WILTONCALL_INTERNAL_HPP

#include <cstdint>
#include <string>

#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"
#include "common/utils.hpp"

namespace wilton {

//// Server
//
//std::string server_create(const std::string& data);
//
//std::string server_stop(const std::string& data);
//
//std::string request_get_metadata(const std::string& data);
//
//std::string request_get_data(const std::string& data);
//
//std::string request_get_data_filename(const std::string& data);
//
//std::string request_set_response_metadata(const std::string& data);
//
//std::string request_send_response(const std::string& data);
//
//std::string request_send_temp_file(const std::string& data);
//
//std::string request_send_mustache(const std::string& data);
//
//std::string request_send_later(const std::string& data);
//
//std::string request_send_with_response_writer(const std::string& data);
//
//// Logger
//
//std::string logger_initialize(const std::string& data);
//
//std::string logger_log(const std::string& data);
//
//std::string logger_is_level_enabled(const std::string& data);
//
//std::string logger_shutdown(const std::string& data);
//
// Mustache
namespace mustache {

std::string mustache_render(const std::string& data);

std::string mustache_render_file(const std::string& data);

} // namespace

//// DB
//
//std::string db_connection_open(const std::string& data);
//
//std::string db_connection_query(const std::string& data);
//
//std::string db_connection_execute(const std::string& data);
//
//std::string db_connection_close(const std::string& data);
//
//std::string db_transaction_start(const std::string& data);
//
//std::string db_transaction_commit(const std::string& data);
//
//std::string db_transaction_rollback(const std::string& data);
//
//// HttpClient
namespace client {

std::string httpclient_create(const std::string& data);

std::string httpclient_close(const std::string& data);

std::string httpclient_execute(const std::string& data);

std::string httpclient_send_temp_file(const std::string& data);

} // namespace

//// Cron
//
//std::string cron_start(const std::string& data);
//
//std::string cron_stop(const std::string& data);
//
//// Mutex
//
//std::string mutex_create(const std::string& data);
//
//std::string mutex_lock(const std::string& data);
//
//std::string mutex_unlock(const std::string& data);
//
//std::string mutex_wait(const std::string& data);
//
//std::string mutex_notify_all(const std::string& data);
//
//std::string mutex_destroy(const std::string& data);
//
// shared
namespace shared {

std::string shared_put(const std::string& data);

std::string shared_get(const std::string& data);

std::string shared_wait_change(const std::string& data);

std::string shared_remove(const std::string& data);

}

//// thread
//
//std::string thread_run(const std::string& data);
//
//std::string thread_sleep_millis(const std::string& data);
//

// misc
namespace misc {

std::string tcp_wait_for_connection(const std::string& data);
    
} // namespace

} // namespace

#endif	/* WILTON_CALL_WILTONCALL_INTERNAL_HPP */

