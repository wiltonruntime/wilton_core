/* 
 * File:   wilton_test.c
 * Author: alex
 *
 * Created on May 6, 2016, 9:44 PM
 */

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

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

void test_server() {
    char* err;
    wilton_Server* server;
    const char* server_conf = "{\"tcpPort\": 8080}";

    wilton_HttpPath* path;
    err = wilton_HttpPath_create(&path, "GET", (int) strlen("GET"), "/", (int) strlen("/"), NULL, hello);
    check_err(err);

    err = wilton_Server_create(&server, server_conf, (int) strlen(server_conf), &path, 1);
    check_err(err);
    wilton_HttpPath_destroy(path);

    //getchar();

    err = wilton_Server_stop(server);
    check_err(err);
}

void test_duktape_fail() {
    const char* in = "{\"module\": \"tests/hello_duktapeX\", \"func\": \"hello\", \"args\": []}";
    char* out;
    int out_len;
    char* err = wiltoncall_runscript_duktape(in, strlen(in), &out, &out_len);
    puts(err);
    wilton_free(err);
}

void runScript(const char* in) {
    char* out;
    int out_len;
    char* err = wiltoncall_runscript_duktape(in, strlen(in), &out, &out_len);
    check_err(err);
    wilton_free(out);
}

void test_wiltonjs() {
    const char* config = "{"
    "  \"requireJsDirPath\": \"../test/js/requirejs\","
    "  \"requireJsConfig\": {"
    "    \"waitSeconds\": 60,"
    "    \"baseUrl\": \"../test/js/modules\""
    "  }"
    "}";
    wiltoncall_init(config, strlen(config));
    runScript("{\"module\": \"tests/runtests\", \"func\": \"runTests\", \"args\": []}");
    runScript("{\"module\": \"inherits/test\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"assert/test\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"util/test/browser/inspect\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"util/test/browser/is\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"underscore/test/arrays\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"underscore/test/chaining\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"underscore/test/collections\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"underscore/test/functions\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"underscore/test/objects\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"underscore/test/utility\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"extend/test/index\", \"func\": \"\", \"args\": []}"); 
    runScript("{\"module\": \"minimist/test/all_bool\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/bool\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/dash\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/default_bool\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/dotted\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/kv_short\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/long\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/num\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/parse\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/parse_modified\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/short\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/stop_early\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/unknown\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"minimist/test/whitespace\", \"func\": \"\", \"args\": []}");
//    following two are very slow
    runScript("{\"module\": \"validator/test/validators\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"validator/test/sanitizers\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"moment/wilton-sanity-test\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"bluebird/wilton-sanity-test\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"sprintf-js/test/test\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"sprintf-js/test/test_validation\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/base\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/childgroups\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/choices\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/conflict\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/constant\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/formatters\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/group\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/nargs\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/optionals\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/parents\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/positionals\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/prefix\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/sub_commands\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"argparse/test/suppress\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"ieee754/test/basic\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"base64-js/test/convert\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"base64-js/test/url-safe\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"is-buffer/test/basic\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/base64\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/basic\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/compare\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/constructor\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/from-string\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/is-buffer\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/methods\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/slice\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/static\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/to-string\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/write\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"buffer/test/write_infinity\", \"func\": \"\", \"args\": []}");    
    runScript("{\"module\": \"iconv-lite/test/main-test\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"qs/test/index\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"node-url/test\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"events/tests/index\", \"func\": \"\", \"args\": []}");
    runScript("{\"module\": \"readable-stream/test/browser\", \"func\": \"\", \"args\": []}");
}

int main() {
//    test_server();
//    test_duktape_fail();
    test_wiltonjs();

    return 0;
}

