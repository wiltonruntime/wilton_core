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
    
// logging

/*
    "logging": {
        "appenders": [{
            "appenderType": "NULL | CONSOLE | FILE | DAILY_ROLLING_FILE",
            "filePath": "path/to/log/file",
            "layout": "%d{%Y-%m-%d %H:%M:%S,%q} [%-5p %-5.5t %-20.20c] %m%n",
            "thresholdLevel": "TRACE | DEBUG | INFO | WARN | ERROR | FATAL"
        }, ... ],
        "loggers": [{
            "name": "my.logger.name",
            "level": "TRACE | DEBUG | INFO | WARN | ERROR | FATAL"
        }, ...]
    }
 */
char* wilton_logger_initialize(
        const char* conf_json,
        int conf_json_len);

char* wilton_logger_log(
        const char* level_name,
        int level_name_len,
        const char* logger_name,
        int logger_name_len,
        const char* message,
        int message_len);

char* wilton_logger_is_level_enabled(
        const char* logger_name,
        int logger_name_len,
        const char* level_name,
        int level_name_len,
        int* res_out
);

char* wilton_logger_shutdown();


// dyload

char* wilton_dyload(
        const char* name,
        int name_len,
        const char* directory,
        int directory_len);


// load

char* wilton_load_resource(
        const char* url,
        int url_len,
        char** contents_out,
        int* contents_out_len);

char* wilton_load_script(
        const char* url,
        int url_len,
        char** contents_out,
        int* contents_out_len);


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
