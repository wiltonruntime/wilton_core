/* 
 * File:   wiltoncall_signal.cpp
 * Author: alex
 *
 * Created on September 22, 2017, 8:03 PM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "wilton/support/misc.hpp"

namespace wilton {
namespace signal {

support::buffer signal_await(sl::io::span<const char>) {
    // call wilton
    char* err = wilton_signal_await();
    if (nullptr != err) {
        support::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::make_empty_buffer();
}

support::buffer signal_fire(sl::io::span<const char>) {
    // call wilton
    char* err = wilton_signal_fire();
    if (nullptr != err) {
        support::throw_wilton_error(err, TRACEMSG(err));
    }
    return support::make_empty_buffer();
}

} // namespace
}
