/* 
 * File:   wilton_logger.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 4:49 PM
 */

#ifndef WILTON_LOGGING_WILTON_LOGGER_HPP
#define WILTON_LOGGING_WILTON_LOGGER_HPP

#include <string>

#include "staticlib/pimpl.hpp"

#include "wilton/support/exception.hpp"

#include "logging/logging_config.hpp"

namespace wilton {
namespace logging {

class wilton_logger : public sl::pimpl::object {
protected:
    /**
     * implementation class
     */
    class impl;
public:
    /**
     * PIMPL-specific constructor
     * 
     * @param pimpl impl object
     */
    PIMPL_CONSTRUCTOR(wilton_logger)

    static void log(const std::string& level_name, const std::string& logger_name, const std::string& message);
    
    static void apply_config(const logging_config& config);
    
    static bool is_enabled_for_level(const std::string& logger_name, const std::string& level_name);
    
    static void shutdown();
    
};

} // namespace
}

#endif /* WILTON_LOGGING_WILTON_LOGGER_HPP */
