/* 
 * File:   cli.cpp
 * Author: alex
 *
 * Created on June 6, 2017, 6:31 PM
 */

#include <iostream>
#include <string>
#include <vector>

#include "staticlib/json.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/tinydir.hpp"

#include "wilton/wiltoncall.h"

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            std::cerr << "ERROR: invalid arguments" << std::endl;
            std::cerr << "Usage: wilton path/to/index.js [arg1 arg2 ...]" << std::endl;
            return 1;
        }

        // arguments
        auto argvec = std::vector<std::string>();
        for (int i = 0; i < argc; ++i) {
            argvec.emplace_back(argv[i]);
        }
        std::replace(argvec.at(0).begin(), argvec.at(0).end(), '\\', '/');
        std::replace(argvec.at(1).begin(), argvec.at(1).end(), '\\', '/');

        // paths
        auto& index = argvec.at(1);
        auto indexpath = sl::tinydir::path(index);
        if (!indexpath.exists()) {
            std::cerr << "ERROR: specified script file not found: [" + index + "]" << std::endl;
            return 1;
        }
        auto modulesdir = sl::utils::strip_filename(index);
        if (modulesdir == index) {
            modulesdir = ".";
        }
        auto indexmod = sl::utils::strip_parent_dir(index);
        if (sl::utils::ends_with(indexmod, ".js")) {
            indexmod.resize(indexmod.length() - 3);
        }

        // current dir
        auto exepath = sl::utils::current_executable_path();
        auto exedir = sl::utils::strip_filename(exepath);
        std::replace(exedir.begin(), exedir.end(), '\\', '/');

        // wilton init
        auto config = sl::json::dumps({
            {"defaultScriptEngine", "duktape"},
            {"requireJsDirPath", exedir + "requirejs"},
            {"requireJsConfig", {
                    {"waitSeconds", 0},
                    {"enforceDefine", true},
                    {"nodeIdCompat", true},
                    {"baseUrl", modulesdir}
                }
            }
        });
        auto err_init = wiltoncall_init(config.c_str(), static_cast<int> (config.length()));
        if (nullptr != err_init) {
            std::cerr << "ERROR: " << err_init << std::endl;
            wilton_free(err_init);
            return 1;
        }

        // index.js input
        auto input = sl::json::dumps({
            {"module", indexmod},
            {"func", "main"},
            {"args", [&argvec] {
                    auto res = std::vector<sl::json::value>();
                    for (auto& st : argvec) {
                        res.emplace_back(st);
                    }
                    return res;
                }()}
        });

        // call index.js
        char* out = nullptr;
        int out_len = 0;
        char* err_run = wiltoncall_runscript_duktape(input.c_str(), static_cast<int> (input.length()), &out, &out_len);
        if (nullptr != err_run) {
            std::cerr << "ERROR: " << err_run << std::endl;
            wilton_free(err_run);
            return 1;
        }
        wilton_free(out);
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

