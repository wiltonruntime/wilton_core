/* 
 * File:   MustacheProcessor.hpp
 * Author: alex
 *
 * Created on May 26, 2016, 2:51 PM
 */

#ifndef WILTON_MUSTACHE_MUSTACHEPROCESSOR_HPP
#define	WILTON_MUSTACHE_MUSTACHEPROCESSOR_HPP

#include <ios>
#include <map>
#include <string>

#include "staticlib/pimpl.hpp"
#include "staticlib/serialization.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace mustache {

class MustacheProcessor : public staticlib::pimpl::PimplObject {
protected:
    /**
     * Implementation class
     */
    class Impl;
public:
    /**
     * PIMPL-specific constructor
     * 
     * @param pimpl impl object
     */
    PIMPL_CONSTRUCTOR(MustacheProcessor)
    
    MustacheProcessor(const std::string& mustache_file_path, staticlib::serialization::JsonValue json,
            const std::map<std::string, std::string>& partials = std::map<std::string, std::string>());
    
    std::streamsize read(char* buffer, std::streamsize length);
    
    static std::string render_string(const std::string& template_text, 
            const staticlib::serialization::JsonValue& json);
   
};

} // namespace
}


#endif	/* WILTON_MUSTACHE_MUSTACHEPROCESSOR_HPP */

