/* 
 * File:   wilton_common.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 7:22 PM
 */

#include "wilton/wilton.h"

#include <cstdlib>

void wilton_free(char* errmsg) {
    std::free(errmsg);
}

