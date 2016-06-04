/* 
 * File:   MustacheProcessor.cpp
 * Author: alex
 * 
 * Created on June 2, 2016, 3:32 PM
 */

#include "mustache/MustacheProcessor.hpp"

#include <array>
#include <map>
#include <vector>

#include "mstch/mstch.hpp"

#include "staticlib/io.hpp"
#include "staticlib/pimpl/pimpl_forward_macros.hpp"
#include "staticlib/utils.hpp"

namespace wilton {
namespace mustache {

namespace { // anonymous

namespace sc = staticlib::config;
namespace si = staticlib::io;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

} //namespace

class MustacheProcessor::Impl : public staticlib::pimpl::PimplObject::Impl {
    mstch::renderer renderer;

public:
    ~Impl() STATICLIB_NOEXCEPT { }
    
    Impl(const std::string& mustache_file_path, ss::JsonValue json) try :
        renderer(read_file(mustache_file_path), create_node(json)) {
    } catch (const std::exception& e) {
        throw common::WiltonInternalException(TRACEMSG(std::string() + e.what() +
                "\nError processing mustache template: [" + mustache_file_path + "]" +
                " with values: [" + ss::dump_json_to_string(json) + "]"));
    }

    std::streamsize read(MustacheProcessor&, char* buffer, std::streamsize length) {
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
        default: throw common::WiltonInternalException(TRACEMSG(std::string() +
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
PIMPL_FORWARD_CONSTRUCTOR(MustacheProcessor, (const std::string&)(ss::JsonValue), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(MustacheProcessor, std::streamsize, read, (char*)(std::streamsize), (), common::WiltonInternalException)

} // namespace
}

