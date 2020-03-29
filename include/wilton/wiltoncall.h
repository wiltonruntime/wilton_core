/*
 * Copyright 2017, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   wiltoncall.h
 * Author: alex
 *
 * Created on January 8, 2017, 1:01 PM
 */

#ifndef WILTONCALL_H
#define WILTONCALL_H

#include "wilton/wilton.h"

#ifdef __cplusplus
extern "C" {
#endif
    
char* wiltoncall(
        const char* call_name,
        int call_name_len,
        const char* json_in,
        int json_in_len,
        char** json_out,
        int* json_out_len);

char* wiltoncall_register(
        const char* call_name,
        int call_name_len,
        void* call_ctx,
        char* (*call_cb)(
                void* call_ctx,
                const char* json_in,
                int json_in_len,
                char** json_out,
                int* json_out_len));

char* wiltoncall_remove(
        const char* call_name,
        int call_name_len);

char* wiltoncall_list(
        char** json_list_out,
        int* json_list_out_len);

char* wiltoncall_init(
        const char* config_json,
        int config_json_len);

// to "override" (implement in client code) this function
// compile with -DWILTON_DISABLE_DEFAULT_RUNSCRIPT
char* wiltoncall_runscript(
        const char* script_engine_name,
        int script_engine_name_len,
        const char* json_in,
        int json_in_len,
        char** json_out,
        int* json_out_len);

#ifdef __cplusplus
}
#endif

#endif /* WILTONCALL_H */

