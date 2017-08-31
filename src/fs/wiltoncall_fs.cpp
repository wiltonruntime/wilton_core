/* 
 * File:   wiltoncall_fs.cpp
 * Author: alex
 *
 * Created on May 27, 2017, 12:58 PM
 */

#include "call/wiltoncall_internal.hpp"

#include <vector>

#include "staticlib/io.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/support.hpp"
#include "staticlib/tinydir.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace fs {

support::buffer fs_append_file(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    auto rcontents = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else if ("data" == name) {
            rcontents = fi.as_string_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    if (rcontents.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'data' not specified"));
    const std::string& path = rpath.get();
    const std::string& contents = rcontents.get();
    // call 
    try {
        auto src = sl::io::string_source(contents);
        auto sink = sl::tinydir::file_sink(path, sl::tinydir::file_sink::open_mode::append);
        sl::io::copy_all(src, sink);
        return support::make_empty_buffer();
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_exists(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& path = rpath.get();
    // call 
    try {
        auto tpath = sl::tinydir::path(path);
        return support::make_json_buffer({
            { "exists", tpath.exists() }
        });
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_mkdir(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& path = rpath.get();
    // call 
    try {
        sl::tinydir::create_directory(path);
        return support::make_empty_buffer();
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_readdir(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& path = rpath.get();
    // call 
    try {
        auto li = sl::tinydir::list_directory(path);
        auto ra = sl::ranges::transform(li, [](const sl::tinydir::path & pa) -> sl::json::value {
            return sl::json::value(pa.filename());
        });
        return support::make_json_buffer(ra.to_vector());
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_read_file(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& path = rpath.get();
    // call 
    try {
        auto src = sl::tinydir::file_source(path);
        auto sink = sl::io::string_sink();
        sl::io::copy_all(src, sink);
        return support::make_string_buffer(sink.get_string());
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_rename(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto roldpath = std::ref(sl::utils::empty_string());
    auto rnewpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("oldPath" == name) {
            roldpath = fi.as_string_nonempty_or_throw(name);
        } else if ("newPath" == name) {
            rnewpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (roldpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'oldPath' not specified"));
    if (rnewpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'newPath' not specified"));    
    const std::string& oldpath = roldpath.get();
    const std::string& newpath = rnewpath.get();
    // call 
    try {
        auto old = sl::tinydir::path(oldpath);
        old.rename(newpath);
        return support::make_empty_buffer();
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_rmdir(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& path = rpath.get();
    // call
    try {
        auto tpath = sl::tinydir::path(path);
        if (tpath.is_directory()) {
            tpath.remove();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Invalid directory path: [" + path + "]"));
        }
        return support::make_empty_buffer();
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_stat(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& path = rpath.get();
    // call
    try {
        auto tpath = sl::tinydir::path(path);
        return support::make_json_buffer({
            { "size", tpath.is_regular_file() ? tpath.open_read().size() : 0 },
            { "isFile", tpath.is_regular_file() },
            { "isDirectory", tpath.is_directory() }
        });
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_unlink(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    const std::string& path = rpath.get();
    // call
    try {
        auto tpath = sl::tinydir::path(path);
        if (tpath.is_regular_file()) {
            tpath.remove();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Invalid file path: [" + path + "]"));
        }
        return support::make_empty_buffer();
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_write_file(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    auto rcontents = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else if ("data" == name) {
            rcontents = fi.as_string_or_throw(name);            
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    if (rcontents.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'data' not specified"));    
    const std::string& path = rpath.get();
    const std::string& contents = rcontents.get();
    // call 
    try {
        auto src = sl::io::string_source(contents);
        auto sink = sl::tinydir::file_sink(path);
        sl::io::copy_all(src, sink);
        return support::make_empty_buffer();
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

support::buffer fs_copy_file(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto roldpath = std::ref(sl::utils::empty_string());
    auto rnewpath = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("oldPath" == name) {
            roldpath = fi.as_string_nonempty_or_throw(name);
        } else if ("newPath" == name) {
            rnewpath = fi.as_string_nonempty_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (roldpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'oldPath' not specified"));
    if (rnewpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'newPath' not specified"));
    const std::string& oldpath = roldpath.get();
    const std::string& newpath = rnewpath.get();
    // call 
    try {
        auto old = sl::tinydir::path(oldpath);
        old.copy_file(newpath);
        return support::make_empty_buffer();
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

} // namespace
}
