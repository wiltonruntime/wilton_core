/* 
 * File:   wiltoncall_load.cpp
 * Author: alex
 *
 * Created on July 17, 2017, 9:03 PM
 */

#include "call/wiltoncall_internal.hpp"

#include "staticlib/tinydir.hpp"

namespace wilton {
namespace load {

namespace { // anonymous

sl::io::span<char> read_zip_resource(const std::string& path) {
    // get modules.zip index
    auto idx_ptr = wilton::internal::static_modules_idx();
    if (!idx_ptr.has_value()) {
        throw wilton::common::wilton_internal_exception(TRACEMSG(
                "Modules index not loaded, requested zip path: [" + path + "]"));
    }
    auto& idx = *idx_ptr;
    // normalize path
    auto path_norm = sl::tinydir::normalize_path(path);
    // load zip entry
    auto& zippath = idx.get_zip_file_path();
    if (path.length() > zippath.length() + 1 && sl::utils::starts_with(path_norm, zippath)) {
        auto en_path = path_norm.substr(zippath.length() + 1);
        auto stream = sl::unzip::open_zip_entry(idx, en_path);
        auto src = sl::io::streambuf_source(stream.get());
        auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free);
        sl::io::copy_all(src, sink);
        return sink.release();
    }
    throw wilton::common::wilton_internal_exception(TRACEMSG("Error loading zip entry," +
            " path: [" + path + "], zip file: [" + zippath + "]"));
}

sl::io::span<char> read_fs_resource(const std::string& path) {
    auto src = sl::tinydir::file_source(path);
    auto sink = sl::io::make_array_sink(wilton_alloc, wilton_free);
    sl::io::copy_all(src, sink);
    return sink.release();
}

sl::io::span<char> read_zip_or_fs_resource(const std::string& url) {
    if (sl::utils::starts_with(url, wilton::internal::file_proto_prefix)) {
        return read_fs_resource(url.substr(wilton::internal::file_proto_prefix.length()));
    } else if (sl::utils::starts_with(url, wilton::internal::zip_proto_prefix)) {
        auto zpath = url.substr(wilton::internal::zip_proto_prefix.length());
        return read_zip_resource(zpath);
    } else {
        throw common::wilton_internal_exception(TRACEMSG("Unknown protocol prefix, path: [" + url + "]"));
    }
}

std::string read_main_from_package_json(const std::string& url) {
    std::string pjurl = std::string(url) + "package.json";
    try {
        auto span = read_zip_or_fs_resource(pjurl);
        auto deferred = sl::support::defer([span] () STATICLIB_NOEXCEPT {
            wilton_free(span.data());
        });
        auto pj = sl::json::load(span);
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

support::buffer load_module_resource(sl::io::span<const char> data) {
    auto url = std::string(data.data(), data.size());
    auto res = read_zip_or_fs_resource(url);
    return sl::support::make_optional(std::move(res));
}

support::buffer load_module_script(sl::io::span<const char> data) {
    try {
        return load_module_resource(data);
    } catch (const std::exception& epath) {
        auto url = std::string(data.data(), data.size());
        if (sl::utils::ends_with(url, ".js")) {
            url.resize(url.length() - 3);
        }
        if (!sl::utils::ends_with(url, "/")) {
            url.push_back('/');
        }
        auto main = read_main_from_package_json(url);
        url.append(main);
        try {
            return load_module_resource(url);
        } catch (const std::exception& etpath) {
            throw common::wilton_internal_exception(TRACEMSG(epath.what() + "\n" + etpath.what()));
        }
    }
}

} // namespace
}
