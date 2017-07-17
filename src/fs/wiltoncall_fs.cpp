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

namespace { // anonymous

// todo: think about the non-throwing fail logic

std::string read_file(const std::string& path) {
    auto src = sl::tinydir::file_source(path);
    auto sink = sl::io::string_sink();
    sl::io::copy_all(src, sink);
    return std::move(sink.get_string());
}

std::string read_zip_entry(const sl::unzip::file_index& idx, const std::string& path) {
    auto enpath = path;
    sl::utils::replace_all(enpath, "/./", "/");
    auto& zippath = idx.get_zip_file_path();
    if (enpath.length() > zippath.length() && sl::utils::starts_with(enpath, zippath)) {
        auto en_path = enpath.substr(zippath.length() + 1);
        auto stream = sl::unzip::open_zip_entry(idx, en_path);
        auto src = sl::io::streambuf_source(stream.get());
        auto sink = sl::io::string_sink();
        sl::io::copy_all(src, sink);
        return std::move(sink.get_string());
    }
    throw wilton::common::wilton_internal_exception(TRACEMSG("Error loading zip entry," +
            " path: [" + path +"], zip file: [" + zippath + "]"));
}

std::string read_file_or_zip_entry(const std::string& path) {
    auto idx_ptr = wilton::internal::static_modules_idx();
    if (idx_ptr.has_value()) {
        try {
            return read_zip_entry(*idx_ptr, path);
        } catch (const std::exception&) {
            return read_file(path);
        }
    } else {
        return read_file(path);
    }
}

void write_file(const std::string& path, const std::string& contents) {
    auto src = sl::io::string_source(contents);
    auto sink = sl::tinydir::file_sink(path);
    sl::io::copy_all(src, sink);
}

std::vector<std::string> list_directory(const std::string& path) {
    auto li = sl::tinydir::list_directory(path);
    auto ra = sl::ranges::transform(li, [](const sl::tinydir::path& pa) -> std::string {
        return pa.filename();
    });
    return ra.to_vector();
}

std::string read_main_from_package_json(const std::string& path) {
    std::string pjpath = std::string(path) + "package.json";
    try {
        auto src = read_file_or_zip_entry(pjpath);
        auto pj = sl::json::loads(src);
        auto main = pj["main"].as_string("index.js");
        if (!sl::utils::ends_with(main, ".js")) {
            main.append(".js");
        }
        return main;
    } catch (const std::exception&) {
        return "index.js";
    }
}

} // namespace


sl::support::optional<sl::io::span<char>> fs_read_file(sl::io::span<const char> data) {
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
        return support::string_span(read_file(path));
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

sl::support::optional<sl::io::span<char>> fs_write_file(sl::io::span<const char> data) {
    // json parse
    auto json = sl::json::load(data);
    auto rpath = std::ref(sl::utils::empty_string());
    auto rcontents = std::ref(sl::utils::empty_string());
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("path" == name) {
            rpath = fi.as_string_nonempty_or_throw(name);
        } else if ("contents" == name) {
            rcontents = fi.as_string_or_throw(name);            
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (rpath.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'path' not specified"));
    if (rcontents.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'contents' not specified"));    
    const std::string& path = rpath.get();
    const std::string& contents = rcontents.get();
    // call 
    try {
        write_file(path, contents);
        return support::empty_span();
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

sl::support::optional<sl::io::span<char>> fs_list_directory(sl::io::span<const char> data) {
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
        auto vec = list_directory(path);
        auto ra = sl::ranges::transform(vec, [](const std::string& st) {
            return sl::json::value(st);
        });
        return support::json_span(ra.to_vector());
    } catch (const std::exception& e) {
        throw common::wilton_internal_exception(TRACEMSG(e.what()));
    }
}

sl::support::optional<sl::io::span<char>> fs_read_module_script(sl::io::span<const char> data) {
    auto path = std::string(data.data(), data.size());
    if (sl::utils::starts_with(path, "file://")) {
        path = path.substr(7);
    }
    try {
        return support::string_span(read_file_or_zip_entry(path));
    } catch (const std::exception& epath) {
        std::string tpath = path;
        if (sl::utils::ends_with(tpath, ".js")) {
            tpath.resize(tpath.length() - 3);
        }
        if (!sl::utils::ends_with(tpath, "/")) {
            tpath.push_back('/');
        }
        auto main = read_main_from_package_json(tpath);
        tpath.append(main);
        try {
            return support::string_span(read_file_or_zip_entry(tpath));
        } catch (const std::exception& etpath) {
            throw common::wilton_internal_exception(TRACEMSG(epath.what() + "\n" + etpath.what()));
        }
    }
}

sl::support::optional<sl::io::span<char>> fs_read_module_resource(sl::io::span<const char> data) {
    auto path = std::string(data.data(), data.size());
    if (sl::utils::starts_with(path, "file://")) {
        path = path.substr(7);
    }
    return support::string_span(read_file_or_zip_entry(path));
}

} // namespace
}
