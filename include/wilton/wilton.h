/* 
 * File:   wilton.h
 * Author: alex
 *
 * Created on April 30, 2016, 11:49 PM
 */

#ifndef WILTON_H
#define WILTON_H

#ifdef __cplusplus
extern "C" {
#endif

// dyload

char* wilton_dyload(
        const char* name,
        int name_len,
        const char* directory,
        int directory_len);

// misc

char* wilton_alloc(
        int size_bytes);

void wilton_free(
        char* buffer);

char* wilton_config(
        char** conf_json_out,
        int* conf_json_len_out);

char* wilton_clean_tls(
        const char* thread_id,
        int thread_id_len);

char* wilton_register_tls_cleaner(
        void* cleaner_ctx,
        void (*cleaner_cb)(
                void* cleaner_ctx,
                const char* thread_id,
                int thread_id_len));

#ifdef __cplusplus
}
#endif

#endif /* WILTON_H */
