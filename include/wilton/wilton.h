/* 
 * File:   wilton.h
 * Author: alex
 *
 * Created on April 30, 2016, 11:49 PM
 */

#ifndef WILTON_H
#define	WILTON_H

#if !defined(WILTON_EXPORT) && defined(WILTON_SHARED)
#  if defined(WILTON_SHARED_EXPORT) && defined(_WIN32)
#    define WILTON_EXPORT __declspec(dllexport)
#  elif defined(_WIN32)
#    define WILTON_EXPORT __declspec(dllimport)
#  elif defined(WILTON_SHARED_EXPORT) 
#    define WILTON_EXPORT __attribute__((visibility("default")))
#  else
#    define WILTON_EXPORT
#  endif
#endif // !WILTON_EXPORT && WILTON_SHARED

#ifdef __cplusplus
extern "C" {
#endif
    
// server

struct wilton_Server;
typedef struct wilton_Server wilton_Server;

struct wilton_Request;
typedef struct wilton_Request wilton_Request;

struct wilton_ResponseWriter;
typedef struct wilton_ResponseWriter wilton_ResponseWriter;

struct wilton_HttpPath;
typedef struct wilton_HttpPath wilton_HttpPath;

WILTON_EXPORT char* wilton_HttpPath_create(
        wilton_HttpPath** http_path_out,
        const char* method,
        int method_len,
        const char* path,
        int path_len,
        void* handler_ctx,
        void (*handler_cb)(
                void* handler_ctx,
                wilton_Request* request));

WILTON_EXPORT char* wilton_HttpPath_destroy(
        wilton_HttpPath* path);

/*
    {
        "numberOfThreads": uint32_t, 
        "tcpPort": uint16_t,
        "ipAddress": "x.x.x.x",
        "ssl": {
            "keyFile": "path/to/file",
            "keyPassword": "pwd",
            "verifyFile": "path/to/file",
            "verifySubjectSubstr": "CN=some_name",
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
        "requestPayload": {
            "tmpDirPath": "path/to/writable/directory",
            "tmpFilenameLength": uint16_t,
            "memoryLimitBytes": uint32_t
        },
        "mustache": {
            "partialsDirs": ["path/to/dir1", "path/to/dir2" ...]
        }
    }
 */
WILTON_EXPORT char* wilton_Server_create(
        wilton_Server** server_out,
        const char* conf_json,
        int conf_json_len,
        wilton_HttpPath** paths,
        int paths_len);

WILTON_EXPORT char* wilton_Server_stop(
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
    "url": "/path/to/hanldler?param1=val1...",
    "pathname": "/path/to/hanldler",
    "query": "param1=val1...",
    "queries": {
        "param1": "val1",
        "param2": "val21,val22"
        ...
    },
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

WILTON_EXPORT char* wilton_Request_get_request_data_filename(
        wilton_Request* request,
        char** filename_out,
        int* filename_len_out);

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

WILTON_EXPORT char* wilton_Request_send_later(
        wilton_Request* request,
        wilton_ResponseWriter** writer_out);

WILTON_EXPORT char* wilton_ResponseWriter_send(
        wilton_ResponseWriter* writer,
        const char* data,
        int data_len);


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
WILTON_EXPORT char* wilton_logger_initialize(
        const char* conf_json,
        int conf_json_len);

WILTON_EXPORT char* wilton_logger_log(
        const char* level_name,
        int level_name_len,
        const char* logger_name,
        int logger_name_len,
        const char* message,
        int message_len);

WILTON_EXPORT char* wilton_logger_is_level_enabled(
        const char* logger_name,
        int logger_name_len,
        const char* level_name,
        int level_name_len,
        int* res_out
);

WILTON_EXPORT char* wilton_logger_shutdown();

// mustache

WILTON_EXPORT char* wilton_render_mustache(
        const char* template_text,
        int template_text_len,
        const char* values_json,
        int values_json_len,
        char** output_text_out,
        int* output_text_len_out);

WILTON_EXPORT char* wilton_render_mustache_file(
        const char* template_file_path,
        int template_file_path_len,
        const char* values_json,
        int values_json_len,
        char** output_text_out,
        int* output_text_len_out);


// HttpClient

struct wilton_HttpClient;
typedef struct wilton_HttpClient wilton_HttpClient;

/*
 {
    "requestQueueMaxSize": uint32_t,
    "fdsetTimeoutMillis": uint32_t,
    "allRequestsPausedTimeoutMillis": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLMOPT_MAX_HOST_CONNECTIONS.html
    "maxHostConnections": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLMOPT_MAX_TOTAL_CONNECTIONS.html
    "maxTotalConnections": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLMOPT_MAXCONNECTS.html
    "maxconnects": uint32_t
 }
 */
WILTON_EXPORT char* wilton_HttpClient_create(
        wilton_HttpClient** http_out,
        const char* conf_json,
        int conf_json_len);

WILTON_EXPORT char* wilton_HttpClient_close(
        wilton_HttpClient* http);

/*

// options implemented manually
 
    "headers": {
        "Header-Name": "header_value",
        ...
    },
    "method": "GET|POST|PUT|DELETE",
    "abortOnConnectError": true,
    "abortOnResponseError": true,
    "maxNumberOfResponseHeaders": uit16_t,
    "consumerThreadWakeupTimeoutMillis": uit16_t,

// general behavior options

    // https://curl.haxx.se/libcurl/c/CURLOPT_HTTP_VERSION.html
    "forceHttp10": false,
    // https://curl.haxx.se/libcurl/c/CURLOPT_NOPROGRESS.html
    "noprogress": true,
    // https://curl.haxx.se/libcurl/c/CURLOPT_NOSIGNAL.html
    "nosignal": true,
    // https://curl.haxx.se/libcurl/c/CURLOPT_FAILONERROR.html
    "failonerror": false,
    // https://curl.haxx.se/libcurl/c/CURLOPT_PATH_AS_IS.html
    "pathAsIs": true,

// TCP options

    // https://curl.haxx.se/libcurl/c/CURLOPT_TCP_NODELAY.html
    "tcpNodelay": false,
    // https://curl.haxx.se/libcurl/c/CURLOPT_TCP_KEEPALIVE.html
    "tcpKeepalive": false,
    // https://curl.haxx.se/libcurl/c/CURLOPT_TCP_KEEPIDLE.html
    "tcpKeepidleSecs": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLOPT_TCP_KEEPINTVL.html
    "tcpKeepintvlSecs": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLOPT_CONNECTTIMEOUT_MS.html
    "connecttimeoutMillis": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLOPT_TIMEOUT_MS.html
    "timeoutMillis": uint32_t,

// HTTP options

    // https://curl.haxx.se/libcurl/c/CURLOPT_BUFFERSIZE.html 
    "buffersizeBytes": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLOPT_ACCEPT_ENCODING.html
    "acceptEncoding": "gzip",
    // https://curl.haxx.se/libcurl/c/CURLOPT_FOLLOWLOCATION.html
    "followlocation": true,
    // https://curl.haxx.se/libcurl/c/CURLOPT_MAXREDIRS.html
    "maxredirs": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLOPT_USERAGENT.html
    // "Mozilla/5.0 (Linux; U; Android 4.2.2; en-us; GT-I9505 Build/JDQ39) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30"
    useragent = "";

// throttling options

    // https://curl.haxx.se/libcurl/c/CURLOPT_MAX_SEND_SPEED_LARGE.html
    "maxSentSpeedLargeBytesPerSecond": uint32_t,
    // https://curl.haxx.se/libcurl/c/CURLOPT_MAX_RECV_SPEED_LARGE.html
    "maxRecvSpeedLargeBytesPerSecond": uint32_t,

// SSL options

    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLCERT.html
    "sslcertFilename": "",
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLCERTTYPE.html
    "sslcertype": "",
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLKEY.html
    "sslkeyFilename": "",
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLKEYTYPE.html
    "sslKeyType": "",
    // https://curl.haxx.se/libcurl/c/CURLOPT_KEYPASSWD.html
    "sslKeypasswd": "".
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSLVERSION.html
    "requireTls": false.
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYHOST.html
    "sslVerifyhost": false,
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYPEER.html
    "sslVerifypeer": false,
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_VERIFYSTATUS.html
    "sslVerifystatus": false,
    // https://curl.haxx.se/libcurl/c/CURLOPT_CAINFO.html
    "cainfoFilename": "",
    // https://curl.haxx.se/libcurl/c/CURLOPT_CRLFILE.html
    "crlfileFilename": "",
    // https://curl.haxx.se/libcurl/c/CURLOPT_SSL_CIPHER_LIST.html
    "sslCipherList": ""
 }
 {    
    // true if connection was successful
    "connectionSuccess": bool,
    "data": "response_data",
    "headers": {
        "Header-Name": "header_value",
        ...
    },
    // https://curl.haxx.se/libcurl/c/CURLINFO_EFFECTIVE_URL.html
    "effectiveUrl": "",
    // https://curl.haxx.se/libcurl/c/CURLINFO_RESPONSE_CODE.html
    "responseCode": long,
    // https://curl.haxx.se/libcurl/c/CURLINFO_TOTAL_TIME.html
    "totalTimeSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_NAMELOOKUP_TIME.html
    "namelookupTimeSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_CONNECT_TIME.html
    "connectTimeSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_APPCONNECT_TIME.html
    "appconnectTimeSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_PRETRANSFER_TIME.html
    "pretransferTimeSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_STARTTRANSFER_TIME.html
    "starttransferTimeSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_REDIRECT_TIME.html
    "redirectTimeSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_REDIRECT_COUNT.html
    "redirectCount": long,
    // https://curl.haxx.se/libcurl/c/CURLINFO_SPEED_DOWNLOAD.html
    "speedDownloadBytesSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_SPEED_UPLOAD.html
    "speedUploadBytesSecs": double,
    // https://curl.haxx.se/libcurl/c/CURLINFO_HEADER_SIZE.html
    "headerSizeBytes": long,
    // https://curl.haxx.se/libcurl/c/CURLINFO_REQUEST_SIZE.html
    "requestSizeBytes": long,
    // https://curl.haxx.se/libcurl/c/CURLINFO_SSL_VERIFYRESULT.html
    "sslVerifyresult": long,
    // https://curl.haxx.se/libcurl/c/CURLINFO_OS_ERRNO.html
    "osErrno": long,
    // https://curl.haxx.se/libcurl/c/CURLINFO_NUM_CONNECTS.html
    "numConnects": long,
    // https://curl.haxx.se/libcurl/c/CURLINFO_PRIMARY_IP.html
    "primaryIp": "",
    // https://curl.haxx.se/libcurl/c/CURLINFO_PRIMARY_PORT.html
    "primaryPort": long,

 }
 */

WILTON_EXPORT char* wilton_HttpClient_execute(
        wilton_HttpClient* http,
        const char* url,
        int url_len,
        const char* request_data,
        int request_data_len,
        const char* request_metadata_json,
        int request_metadata_len,
        char** response_data_out,
        int* response_data_len_out);

WILTON_EXPORT char* wilton_HttpClient_send_file(
        wilton_HttpClient* http,
        const char* url,
        int url_len,
        const char* file_path,
        int file_path_len,       
        const char* request_metadata_json,
        int request_metadata_len,
        char** response_data_out,
        int* response_data_len_out,
        void* finalizer_ctx,
        void (*finalizer_cb)(
                void* finalizer_ctx,
                int sent_successfully));


// Cron

struct wilton_CronTask;
typedef struct wilton_CronTask wilton_CronTask;

WILTON_EXPORT char* wilton_CronTask_start(
        wilton_CronTask** cron_out,
        const char* cronexpr,
        int cronexpr_len,
        void* task_ctx,
        void (*task_cb)(
                void* task_ctx));


WILTON_EXPORT char* wilton_CronTask_stop(
        wilton_CronTask* cron);

// shared

WILTON_EXPORT char* wilton_shared_put(
        const char* key,
        int key_len,
        const char* value,
        int value_len,
        char** prev_value_out,
        int* prev_value_out_len);

WILTON_EXPORT char* wilton_shared_get(
        const char* key,
        int key_len,
        char** value_out,
        int* value_out_len);

WILTON_EXPORT char* wilton_shared_wait_change(
        int timeout_millis,
        const char* key,
        int key_len,
        const char* current_value,
        int current_value_len,
        char** changed_value_out,
        int* changed_value_out_len);

WILTON_EXPORT char* wilton_shared_remove(
        const char* key,
        int key_len,
        char** value_out,
        int* value_out_len);

WILTON_EXPORT char* wilton_shared_list_append(
        const char* key,
        int key_len,
        const char* value,
        int value_len,
        char** updated_value_out,
        int* updated_value_out_len);

WILTON_EXPORT char* wilton_shared_dump(
        char** dump_out,
        int* dump_out_len);

// thread

WILTON_EXPORT char* wilton_thread_run(
        void* cb_ctx,
        void (*cb)(
                void* cb_ctx));

WILTON_EXPORT char* wilton_thread_sleep_millis(
        int millis);

WILTON_EXPORT char* wilton_thread_wait_for_signal();

WILTON_EXPORT char* wilton_thread_signal_waiters_count(int* count_out);

// misc

WILTON_EXPORT char* wilton_alloc(
        int size_bytes);

WILTON_EXPORT void wilton_free(
        char* buffer);

WILTON_EXPORT char* wilton_tcp_wait_for_connection(
        const char* ip_addr,
        int ip_addr_len,
        int tcp_port,
        int timeout_millis);

#ifdef __cplusplus
}
#endif

#endif	/* WILTON_H */
