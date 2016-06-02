/* 
 * File:   MustacheProcessor.hpp
 * Author: alex
 *
 * Created on May 26, 2016, 2:51 PM
 */

#ifndef WILTON_C_MUSTACHEPROCESSOR_HPP
#define	WILTON_C_MUSTACHEPROCESSOR_HPP

#include <array>
#include <ios>
#include <map>
#include <string>
#include <vector>

#include "mstch/mstch.hpp"

#include "staticlib/io.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

#include "WiltonInternalException.hpp"

namespace wilton {

namespace { // anonymous

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

} //namespace

class MustacheProcessor {
    mstch::renderer renderer;
    
public:
    MustacheProcessor(const MustacheProcessor&) = delete;
    
    MustacheProcessor(MustacheProcessor&& other) :
    renderer(std::move(other.renderer)) { }
    
    MustacheProcessor& operator=(const MustacheProcessor&) = delete;
    
    MustacheProcessor& operator=(MustacheProcessor&& other) {
        this->renderer = std::move(other.renderer);
        return *this;
    }

    MustacheProcessor(const std::string& mustache_file_path, ss::JsonValue json) try :
        renderer(read_file(mustache_file_path), create_node(json)) {
    } catch (const std::exception& e) {
        throw WiltonInternalException(TRACEMSG(std::string() + e.what() +
                "\nError processing mustache template: [" + mustache_file_path + "]" +
                " with values: [" + ss::dump_json_to_string(json) + "]"));
    }
    
    std::streamsize read(char* buffer, std::streamsize length) {
        return renderer.read(buffer, length);
    }
    
private:

    static std::string read_file(const std::string path) {
        su::FileDescriptor fd{path, 'r'};
        std::array<char, 4096> buf;
        si::string_sink sink{};
        si::copy_all(fd, sink, buf.data(), buf.size());
        return std::move(sink.get_string());
    }

    static mstch::node create_node(const ss::JsonValue& value) {
        switch (value.get_type()) {
        case (ss::JsonType::NULL_T): return mstch::node();
        case (ss::JsonType::OBJECT): return create_map(value);
        case (ss::JsonType::ARRAY): return create_array(value);
        case (ss::JsonType::STRING): return mstch::node(value.get_string());
        case (ss::JsonType::INTEGER): return mstch::node(static_cast<int> (value.get_integer()));
        case (ss::JsonType::REAL): return mstch::node(value.get_real());
        case (ss::JsonType::BOOLEAN): return mstch::node(value.get_boolean());
        default: throw WiltonInternalException(TRACEMSG(std::string() +
                    "Unsupported JSON type:[" + sc::to_string(static_cast<char> (value.get_type())) + "]"));
        }
    }    

    static mstch::node create_map(const ss::JsonValue& value) {
        std::map<const std::string, mstch::node> map;
        for (const auto& fi : value.get_object()) {
            map.insert({fi.get_name(), create_node(fi.get_value())});
        }
        return mstch::node(std::move(map));
    }

    static mstch::node create_array(const ss::JsonValue& value) {
        std::vector<mstch::node> array;
        for (const auto& va : value.get_array()) {
            array.emplace_back(create_node(va));
        }
        return mstch::node(std::move(array));
    }
   
};

} // namespace


#endif	/* WILTON_C_MUSTACHEPROCESSOR_HPP */

