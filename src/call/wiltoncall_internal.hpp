/* 
 * File:   wiltoncall_internal.hpp
 * Author: alex
 *
 * Created on January 10, 2017, 11:42 AM
 */

#ifndef WILTON_CALL_WILTONCALL_INTERNAL_HPP
#define	WILTON_CALL_WILTONCALL_INTERNAL_HPP

#include <cstdint>
#include <thread>
#include <string>

#include "staticlib/utils.hpp"
#include "staticlib/json.hpp"
#include "staticlib/io.hpp"

#include "wilton/wiltoncall.h"

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"

namespace wilton {

// Server
namespace server {

std::string server_create(sl::io::span<const char> data);

std::string server_stop(sl::io::span<const char> data);

std::string request_get_metadata(sl::io::span<const char> data);

std::string request_get_data(sl::io::span<const char> data);

std::string request_get_data_filename(sl::io::span<const char> data);

std::string request_set_response_metadata(sl::io::span<const char> data);

std::string request_send_response(sl::io::span<const char> data);

std::string request_send_temp_file(sl::io::span<const char> data);

std::string request_send_mustache(sl::io::span<const char> data);

std::string request_send_later(sl::io::span<const char> data);

std::string request_send_with_response_writer(sl::io::span<const char> data);

} // namespace

// Logger
namespace logging {

std::string logging_initialize(sl::io::span<const char> data);

std::string logging_log(sl::io::span<const char> data);

std::string logging_is_level_enabled(sl::io::span<const char> data);

std::string logging_shutdown(sl::io::span<const char> data);

} // namespace

// Mustache
namespace mustache {

std::string mustache_render(sl::io::span<const char> data);

std::string mustache_render_file(sl::io::span<const char> data);

} // namespace

// HttpClient
namespace client {

std::string httpclient_create(sl::io::span<const char> data);

std::string httpclient_close(sl::io::span<const char> data);

std::string httpclient_execute(sl::io::span<const char> data);

std::string httpclient_send_temp_file(sl::io::span<const char> data);

} // namespace

// Cron
namespace cron {

std::string cron_start(sl::io::span<const char> data);

std::string cron_stop(sl::io::span<const char> data);

} // namespace

// shared
namespace shared {

std::string shared_put(sl::io::span<const char> data);

std::string shared_get(sl::io::span<const char> data);

std::string shared_wait_change(sl::io::span<const char> data);

std::string shared_remove(sl::io::span<const char> data);

} // namespace

// thread
namespace thread {

std::string thread_run(sl::io::span<const char> data);

std::string thread_sleep_millis(sl::io::span<const char> data);

} // namespace

namespace fs {

std::string fs_read_file(sl::io::span<const char> data);

std::string fs_write_file(sl::io::span<const char> data);

std::string fs_list_directory(sl::io::span<const char> data);

std::string fs_read_script_file_or_module(sl::io::span<const char> data);

} // namespace

// dyload

namespace dyload {

std::string dyload_shared_library(sl::io::span<const char> data);

} // namespace

// misc
namespace misc {

std::string tcp_wait_for_connection(sl::io::span<const char> data);
    
} // namespace


// internal api

namespace internal {

const sl::json::value& static_wiltoncall_config(const std::string& cf_json = "");

void clean_duktape_thread_local(const std::thread::id& tid);

} // namespace

} // namespace

#endif	/* WILTON_CALL_WILTONCALL_INTERNAL_HPP */

