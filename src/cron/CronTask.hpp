/* 
 * File:   CronTask.hpp
 * Author: alex
 *
 * Created on September 7, 2016, 12:51 PM
 */

#ifndef WILTON_CRON_CRONTASK_HPP
#define	WILTON_CRON_CRONTASK_HPP

#include <functional>
#include <string>

#include "staticlib/pimpl.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace cron {

class CronTask : public staticlib::pimpl::pimpl_object {
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
    PIMPL_CONSTRUCTOR(CronTask)

    CronTask(const std::string& cronexpr, std::function<void()> task);

    void stop();

};

} // namespace
}
#endif	/* WILTON_CRON_CRONTASK_HPP */

