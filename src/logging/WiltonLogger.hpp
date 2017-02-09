/* 
 * File:   WiltonLogger.hpp
 * Author: alex
 *
 * Created on May 10, 2016, 4:49 PM
 */

#ifndef WILTON_LOGGING_WILTONLOGGER_HPP
#define	WILTON_LOGGING_WILTONLOGGER_HPP

#include <string>

#include "staticlib/pimpl.hpp"

#include "common/WiltonInternalException.hpp"
#include "logging/LoggingConfig.hpp"

namespace wilton {
namespace logging {

class WiltonLogger : public staticlib::pimpl::pimpl_object {
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
    PIMPL_CONSTRUCTOR(WiltonLogger)

    static void log(const std::string& level_name, const std::string& logger_name, const std::string& message);
    
    static void apply_config(const LoggingConfig& config);
    
    static bool is_enabled_for_level(const std::string& logger_name, const std::string& level_name);
    
    static void shutdown();
    
};

} // namespace
}

#endif	/* WILTON_LOGGING_WILTONLOGGER_HPP */
