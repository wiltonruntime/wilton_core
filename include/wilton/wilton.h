/* 
 * File:   wilton.h
 * Author: alex
 *
 * Created on April 30, 2016, 11:49 PM
 */

#ifndef WILTON_H
#define	WILTON_H

#ifdef WILTON_SHARED
#ifdef _WIN32
#ifdef WILTON_SHARED_IMPORT
#define WILTON_EXPORT __declspec(dllimport)
#else
#define WILTON_EXPORT __declspec(dllexport)
#endif // WILTON_SHARED_IMPORT
#else
#ifdef WILTON_SHARED_IMPORT
#define WILTON_EXPORT
#else
#define WILTON_EXPORT __attribute__ ((visibility ("default")))
#endif // WILTON_SHARED_IMPORT
#endif // _WIN32
#else
#define WILTON_EXPORT
#endif // WILTON_SHARED

#ifdef	__cplusplus
extern "C" {
#endif

struct wilton_Server;
typedef struct wilton_Server wilton_Server;

struct wilton_Request;
typedef struct wilton_Request wilton_Request;

WILTON_EXPORT void wilton_free(
        char* errmsg);

WILTON_EXPORT char* wilton_log(
        const char* level_name,
        int level_name_len,        
        const char* logger_name,
        int logger_name_len,
        const char* message,
        int message_len);

/*
    {
        "numberOfThreads": uint32_t, 
        "tcpPort": uint16_t,
        "ipAddress": "x.x.x.x",
        "ssl": {
            "keyFile": "path/to/file",
            "keyPassword": "pwd",
            "verifyFile": "path/to/file",
            "verifyOrganization_unit": "ou_name",
        },
        "documentRoots": [{
            "resource": "/path/to/hanldler",
            "dirPath": "path/to/directory",
            "zipPath": "path/to/directory.zip",
            "zipInnerPrefix": "some/dir",
            "cacheMaxAgeSeconds": uint32_t,
            "mimeTypes": [{
                "extension": ".css",
                "mime": "text/css"
            }, ...]
        }, ...], 
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
    }
 */
WILTON_EXPORT char* wilton_Server_create(
        wilton_Server** server_out,
        void* gateway_ctx,
        void (*gateway_cb)(
                void* gateway_ctx,
                wilton_Request* request),
        const char* conf_json,
        int conf_json_len);

WILTON_EXPORT char* wilton_Server_stop_server(
        wilton_Server* server);

/*
// Duplicates in raw headers are handled in the following ways, depending on the header name:
// Duplicates of age, authorization, content-length, content-type, etag, expires, 
// from, host, if-modified-since, if-unmodified-since, last-modified, location, 
// max-forwards, proxy-authorization, referer, retry-after, or user-agent are discarded.
// For all other headers, the values are joined together with ', '.
{
    "httpVersion": "1.1",
    "protocol": "http|https",
    "method": "GET|POST|PUT|DELETE",
    "pathname": "/path/to/hanldler",
    "query": "param1=val1...",
    "url": "/path/to/hanldler?param1=val1...",
    "headers": {
        "Header-Name": "header_value",
        ...
    }
}
 */
WILTON_EXPORT char* wilton_Request_get_request_metadata(
        wilton_Request* request,
        char** metadata_json_out,
        int* metadata_json_len_out);

WILTON_EXPORT char* wilton_Request_get_request_data(
        wilton_Request* request,
        char** data_out,
        int* data_len_out);

/*
{
    "statusCode": uint16_t,
    "statusMessage": "Status message",
    "headers": {
        "Header-Name": "header_value",
        ...
    }
}
 */
WILTON_EXPORT char* wilton_Request_set_response_metadata(
        wilton_Request* request,
        const char* metadata_json,
        int metadata_json_len);

WILTON_EXPORT char* wilton_Request_send_response(
        wilton_Request* request,
        const char* data,
        int data_len);

WILTON_EXPORT char* wilton_Request_send_file(
        wilton_Request* request,
        const char* file_path,
        int file_path_len,
        void* finalizer_ctx,
        void (*finalizer_cb)(
                void* finalizer_ctx,
                int sent_successfully));

WILTON_EXPORT char* wilton_Request_send_mustache(
        wilton_Request* request,
        const char* mustache_file_path,
        int mustache_file_path_len,
        const char* values_json,
        int values_json_len);

#ifdef	__cplusplus
}
#endif

#endif	/* WILTON_H */
