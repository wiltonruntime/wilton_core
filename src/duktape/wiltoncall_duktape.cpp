/* 
 * File:   wiltoncall_duktape.cpp
 * Author: alex
 *
 * Created on May 20, 2017, 1:17 PM
 */

#include "call/wiltoncall_internal.hpp"

#include "staticlib/config.hpp"

char* wiltoncall_runscript_duktape(const char* json_in, int json_in_len, char** json_out,
        int* json_out_len) /* noexcept */ {
    (void) json_in;
    (void) json_in_len;
    (void) json_out;
    (void) json_out_len;
    return sl::utils::alloc_copy(TRACEMSG("Duktape engine not implemented"));
}
