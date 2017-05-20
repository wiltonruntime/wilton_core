/* 
 * File:   wiltoncall.h
 * Author: alex
 *
 * Created on January 8, 2017, 1:01 PM
 */

#ifndef WILTONCALL_H
#define	WILTONCALL_H

#include "wilton/wilton.h"

#ifdef	__cplusplus
extern "C" {
#endif

WILTON_EXPORT char* wiltoncall_init(
        const char* default_script_engine_name,
        int default_script_engine_name_len);
    
WILTON_EXPORT char* wiltoncall(
        const char* call_name,
        int call_name_len,
        const char* json_in,
        int json_in_len,
        char** json_out,
        int* json_out_len);

WILTON_EXPORT char* wiltoncall_runscript(
        const char* script_engine_name,
        int script_engine_name_len,
        const char* json_in,
        int json_in_len,
        char** json_out,
        int* json_out_len);

WILTON_EXPORT char* wiltoncall_register(
        const char* call_name,
        int call_name_len,
        void* call_ctx,
        char* (*call_cb)(
                void* call_ctx,
                const char* json_in,
                int json_in_len,
                char** json_out,
                int* json_out_len));

WILTON_EXPORT char* wiltoncall_remove(
        const char* call_name,
        int call_name_len);

#ifdef	__cplusplus
}
#endif

#endif	/* WILTONCALL_H */

