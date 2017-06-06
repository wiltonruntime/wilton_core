/* 
 * File:   wilton_test_module.c
 * Author: alex
 *
 * Created on June 6, 2017, 8:00 PM
 */

#include <stdio.h>

#ifdef _WIN32
__declspec(dllexport)
#endif
char* wilton_module_init() {
    puts("Hi from wilton_module_init!");
    return NULL;
}

