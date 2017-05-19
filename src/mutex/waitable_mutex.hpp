/* 
 * File:   waitable_mutex.hpp
 * Author: alex
 *
 * Created on October 4, 2016, 7:21 PM
 */

#ifndef WILTON_MUTEX_WAITABLE_MUTEX_HPP
#define	WILTON_MUTEX_WAITABLE_MUTEX_HPP

#include "staticlib/pimpl.hpp"

#include <functional>

#include "common/wilton_internal_exception.hpp"

namespace wilton {
namespace mutex {

class waitable_mutex : public sl::pimpl::object {
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
    PIMPL_CONSTRUCTOR(waitable_mutex)
            
    waitable_mutex();
            
    void lock();

    void unlock();
    
    void wait(uint32_t timeout_millis, std::function<bool()> cond);
    
    void notify_all();

};

} // namespace
}

#endif	/* WILTON_MUTEX_WAITABLE_MUTEX_HPP */

