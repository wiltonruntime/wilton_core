/* 
 * File:   Mutex.hpp
 * Author: alex
 *
 * Created on October 4, 2016, 7:21 PM
 */

#ifndef WILTON_MUTEX_MUTEX_HPP
#define	WILTON_MUTEX_MUTEX_HPP

#include "staticlib/pimpl.hpp"

#include "common/WiltonInternalException.hpp"

namespace wilton {
namespace mutex {

class Mutex : public staticlib::pimpl::PimplObject {
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
    PIMPL_CONSTRUCTOR(Mutex)
            
    Mutex();
            
    void lock();

    void unlock();

};

} // namespace
}

#endif	/* WILTON_MUTEX_MUTEX_HPP */

