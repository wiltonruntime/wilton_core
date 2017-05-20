/* 
 * File:   utils.cpp
 * Author: alex
 * 
 * Created on June 14, 2016, 11:45 AM
 */

#include "common/utils.hpp"

#include "staticlib/io.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"

#include "wilton/wilton.h"

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace common {

void throw_wilton_error(char* err, const std::string& msg) {
    wilton_free(err);
    throw wilton_internal_exception(msg);
}

std::string wrap_wilton_output(char* out, int out_len) {
    std::string res{out, static_cast<std::string::size_type> (out_len)};
    wilton_free(out);
    return res;
}

void check_json_callback_script(const sl::json::field& field) {
    if (sl::json::type::object != field.json_type()) {
        throw common::wilton_internal_exception(TRACEMSG("Invalid '" + field.name() + "' field,"
                " type: [" + sl::json::stringify_json_type(field.json_type()) + "]," +
                " value: [" + field.val().dumps() + "]"));
    }
    bool module = false;
    bool func = false;
    bool args = false;
    for (const sl::json::field& fi : field.as_object()) {
        auto& name = fi.name();
        if ("module" == name) {
            if (sl::json::type::string != fi.json_type()) {
                throw common::wilton_internal_exception(TRACEMSG("Invalid '" + fi.name() + "' field,"
                        " type: [" + sl::json::stringify_json_type(fi.json_type()) + "]," +
                        " value: [" + fi.val().dumps() + "]"));
            }
            module = true;
        } else if ("func" == name) {
            if (sl::json::type::string != fi.json_type()) {
                throw common::wilton_internal_exception(TRACEMSG("Invalid '" + fi.name() + "' field,"
                        " type: [" + sl::json::stringify_json_type(fi.json_type()) + "]," +
                        " value: [" + fi.val().dumps() + "]"));
            }
            func = true;
        } else if ("args" == name) {
            if (sl::json::type::array != fi.json_type()) {
                throw common::wilton_internal_exception(TRACEMSG("Invalid '" + fi.name() + "' field,"
                        " type: [" + sl::json::stringify_json_type(fi.json_type()) + "]," +
                        " value: [" + fi.val().dumps() + "]"));
            }
            args = true;
        } else if ("engine" == name) {
            if (sl::json::type::string != fi.json_type()) {
                throw common::wilton_internal_exception(TRACEMSG("Invalid '" + fi.name() + "' field,"
                        " type: [" + sl::json::stringify_json_type(fi.json_type()) + "]," +
                        " value: [" + fi.val().dumps() + "]"));
            }
        } else {
            throw common::wilton_internal_exception(TRACEMSG(
                    "Unknown data field: [" + name + "] in object: [" + field.name() + "]"));
        }
    }
    if (!module) {
        throw common::wilton_internal_exception(TRACEMSG(
                "Required field: 'module' is not supplied in object: [" + field.name() + "]"));
    }
    if (!func) {
        throw common::wilton_internal_exception(TRACEMSG(
                "Required field: 'func' is not supplied in object: [" + field.name() + "]"));
    }
    if (!args) {
        throw common::wilton_internal_exception(TRACEMSG(
                "Required field: 'func' is not supplied in object: [" + field.name() + "]"));
    }
}

void dump_error(const std::string& directory, const std::string& msg) {
    try {
        // random postfix
        std::string id = sl::utils::random_string_generator().generate(12);
        auto errfile = directory + "wilton_ERROR_" + id + ".txt";
        auto fd = sl::tinydir::file_sink(errfile);
        sl::io::write_all(fd, msg);
    } catch (...) {
        // give up
    }
}

} //namespace
}  

