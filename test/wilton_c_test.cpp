/* 
 * File:   wilton_c_test.cpp
 * Author: alex
 *
 * Created on May 6, 2016, 9:44 PM
 */

// C++ used here instead of C to support static linking
// dynamic linking will work with plain C

#include "wilton/wilton_c.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

void hello(void* ctx, wilton_Request* req) {
    (void) ctx;
    wilton_Request_send_response(req, "hello\n", 6);
}

void check_err(char* err) {
    if (NULL != err) {
        puts(err);
        exit(1);
    }
}

int main() {
    char* err;
    wilton_Server* server;
    const char* server_conf = "{\"tcpPort\": 8080}";
    err = wilton_Server_create(&server, NULL, hello, server_conf, strlen(server_conf));
    check_err(err);

    getchar();

    err = wilton_Server_stop_server(server);
    check_err(err);

    return 0;
}

