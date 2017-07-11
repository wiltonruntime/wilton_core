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

#include "staticlib/json.hpp"
#include "staticlib/io.hpp"
#include "staticlib/support.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wiltoncall.h"
#include "wilton/support/handle_registry.hpp"
#include "wilton/support/payload_handle_registry.hpp"
#include "wilton/support/span_operations.hpp"

#include "common/wilton_internal_exception.hpp"
#include "common/utils.hpp"

namespace wilton {

// Server
namespace server {

sl::support::optional<sl::io::span<char>> server_create(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> server_stop(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_get_metadata(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_get_data(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_get_data_filename(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_set_response_metadata(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_send_response(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_send_temp_file(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_send_mustache(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_send_later(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> request_send_with_response_writer(sl::io::span<const char> data);

} // namespace

// Logger
namespace logging {

sl::support::optional<sl::io::span<char>> logging_initialize(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> logging_log(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> logging_is_level_enabled(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> logging_shutdown(sl::io::span<const char> data);

} // namespace

// Mustache
namespace mustache {

sl::support::optional<sl::io::span<char>> mustache_render(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> mustache_render_file(sl::io::span<const char> data);

} // namespace

// HttpClient
namespace client {

sl::support::optional<sl::io::span<char>> httpclient_send_request(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> httpclient_send_temp_file(sl::io::span<const char> data);

} // namespace

// Cron
namespace cron {

sl::support::optional<sl::io::span<char>> cron_start(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> cron_stop(sl::io::span<const char> data);

} // namespace

// shared
namespace shared {

sl::support::optional<sl::io::span<char>> shared_put(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> shared_get(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> shared_wait_change(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> shared_remove(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> shared_list_append(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> shared_dump(sl::io::span<const char> data);

} // namespace

// thread
namespace thread {

sl::support::optional<sl::io::span<char>> thread_run(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> thread_sleep_millis(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> thread_wait_for_signal(sl::io::span<const char> data);

} // namespace

namespace fs {

sl::support::optional<sl::io::span<char>> fs_read_file(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> fs_write_file(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> fs_list_directory(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> fs_read_script_file_or_module(sl::io::span<const char> data);

} // namespace

// dyload

namespace dyload {

sl::support::optional<sl::io::span<char>> dyload_shared_library(sl::io::span<const char> data);

} // namespace

// misc
namespace misc {

sl::support::optional<sl::io::span<char>> tcp_wait_for_connection(sl::io::span<const char> data);

sl::support::optional<sl::io::span<char>> process_spawn(sl::io::span<const char> data);
    
} // namespace


// internal api

namespace internal {

const sl::json::value& static_wiltoncall_config(const std::string& cf_json = "");

void clean_duktape_thread_local(const std::thread::id& tid);

} // namespace

} // namespace

#endif	/* WILTON_CALL_WILTONCALL_INTERNAL_HPP */

