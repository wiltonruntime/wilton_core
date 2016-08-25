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
        throw common::WiltonInternalException(TRACEMSG(e.what() +
                "\nError processing mustache template: [" + mustache_file_path + "]" +
                " with values: [" + ss::dump_json_to_string(json) + "]"));
    }

    std::streamsize read(MustacheProcessor&, char* buffer, std::streamsize length) {
        return renderer.read(buffer, length);
    }

    static std::string render_string(const std::string& template_text, const ss::JsonValue& json) {
        mstch::node node = create_node(json);
        return mstch::render(template_text, node);
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
        switch (value.type()) {
        case (ss::JsonType::NULL_T): return mstch::node();
        case (ss::JsonType::OBJECT): return create_map(value);
        case (ss::JsonType::ARRAY): return create_array(value);
        case (ss::JsonType::STRING): return mstch::node(value.as_string());
        case (ss::JsonType::INTEGER): return mstch::node(static_cast<int> (value.as_int64()));
        case (ss::JsonType::REAL): return mstch::node(value.as_double());
        case (ss::JsonType::BOOLEAN): return mstch::node(value.as_bool());
        default: throw common::WiltonInternalException(TRACEMSG(
                    "Unsupported JSON type:[" + sc::to_string(static_cast<char> (value.type())) + "]"));
        }
    }

    static mstch::node create_map(const ss::JsonValue& value) {
        std::map<const std::string, mstch::node> map;
        for (const auto& fi : value.as_object()) {
            map.insert({fi.name(), create_node(fi.value())});
        }
        return mstch::node(std::move(map));
    }

    static mstch::node create_array(const ss::JsonValue& value) {
        std::vector<mstch::node> array;
        for (const auto& va : value.as_array()) {
            array.emplace_back(create_node(va));
        }
        return mstch::node(std::move(array));
    }

};
PIMPL_FORWARD_CONSTRUCTOR(MustacheProcessor, (const std::string&)(ss::JsonValue), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD(MustacheProcessor, std::streamsize, read, (char*)(std::streamsize), (), common::WiltonInternalException)
PIMPL_FORWARD_METHOD_STATIC(MustacheProcessor, std::string, render_string, (const std::string&)(const ss::JsonValue&), (), common::WiltonInternalException)

} // namespace
}

