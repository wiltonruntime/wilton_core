/* 
 * File:   cron_task.hpp
 * Author: alex
 *
 * Created on September 7, 2016, 12:51 PM
 */

#ifndef WILTON_CRON_CRON_TASK_HPP
#define WILTON_CRON_CRON_TASK_HPP

#include <functional>
#include <string>

#include "staticlib/pimpl.hpp"

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace cron {

class cron_task : public sl::pimpl::object {
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
    PIMPL_CONSTRUCTOR(cron_task)

    cron_task(const std::string& cronexpr, std::function<void()> task);

    void stop();

};

} // namespace
}
#endif /* WILTON_CRON_CRON_TASK_HPP */

