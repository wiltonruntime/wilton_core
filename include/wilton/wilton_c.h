/* 
 * File:   wilton_c.h
 * Author: alex
 *
 * Created on April 30, 2016, 11:49 PM
 */

#ifndef WILTON_C_H
#define	WILTON_C_H

#ifdef _WIN32
#ifdef WILTON_DLL_IMPORT
#define WILTON_EXPORT __declspec(dllimport)
#else
#define WILTON_EXPORT __declspec(dllexport)
#endif
#else
#define WILTON_EXPORT __attribute__ ((visibility ("default")))
#endif

#ifdef	__cplusplus
extern "C" {
#endif

struct wilton_Server;
typedef struct wilton_Server wilton_Server;

struct wilton_Request;
typedef struct wilton_Request wilton_Request;

WILTON_EXPORT void wilton_free(
        char* errmsg);

WILTON_EXPORT char* wilton_Server_create(
        wilton_Server** server_out,
        void* handler_provider_ctx,
        int (*handler_provider_cb)(
                void* ctx,
                const char* handler_name,
                int handler_name_len,
                void** handler_ctx_out,
                (*handler_out)(
                        void* ctx,
                        wilton_Request* request)*),
        const char* conf_json,
        int conf_json_len);

WILTON_EXPORT char* wilton_Server_stop_server(
        wilton_Server* server);

WILTON_EXPORT char* wilton_Request_get_request_metadata(
        wilton_Request* request,
        const char** metadata_json_out,
        int* metadata_json_len_out);

WILTON_EXPORT const char* wilton_Request_get_request_data(
        wilton_Request* request,
        const char** data_out,
        int* data_len_out);

WILTON_EXPORT char* wilton_Request_set_response_metadata(
        wilton_Request* request,
        const char* metadata_json,
        int metadata_json_len);

WILTON_EXPORT char* wilton_Request_send_response(
        wilton_Request* request,
        const char* data,
        int data_len);

WILTON_EXPORT char* wilton_Request_send_response_chunked(
        wilton_Request* request,
        void* read_ctx,
        int (*read)(
                void* read_ctx,
                char* buf,
                int len));

#ifdef	__cplusplus
}
#endif

#endif	/* WILTON_C_H */

