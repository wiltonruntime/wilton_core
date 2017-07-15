/* 
 * File:   cli.cpp
 * Author: alex
 *
 * Created on June 6, 2017, 6:31 PM
 */

#include <iostream>
#include <string>
#include <vector>

#include <popt.h>

#include "staticlib/json.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/tinydir.hpp"

#include "wilton/wiltoncall.h"

#include "cli_options.hpp"

namespace { // anonymous

int find_launcher_args_end(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        if ("--" ==  std::string(argv[i])) {
            return i;
        }
    }
    return argc;
}

std::string find_exedir() {
    auto exepath = sl::utils::current_executable_path();
    auto exedir = sl::utils::strip_filename(exepath);
    std::replace(exedir.begin(), exedir.end(), '\\', '/');
    return exedir;
}

std::string find_startup_module(const std::string& opts_startup_module_name, const std::string& idxfile_or_dir) {
    if (!opts_startup_module_name.empty()) {
        return opts_startup_module_name;
    }
    auto sm = sl::utils::strip_parent_dir(idxfile_or_dir);
    if (sl::utils::ends_with(sm, ".js")) {
        sm.resize(sm.length() - 3);
    }
    return sm;
}

std::string find_statup_module_path(const std::string& idxfile_or_dir) {
    auto smp = idxfile_or_dir;
    if (sl::utils::ends_with(smp, ".js")) {
        smp.resize(smp.length() - 3);
    }
    return smp;
}

} // namespace

// valgrind run:
// valgrind --leak-check=yes --show-reachable=yes --track-origins=yes --error-exitcode=42 --track-fds=yes --suppressions=../../../staticlibs/cmake/resources/valgrind/openssl_malloc.supp  ./bin/wilton ../../modules/runWiltonTests.js -r ../../wilton-requirejs/

int main(int argc, char** argv) {    
    try {
        // parse laucher args
        int launcher_argc = find_launcher_args_end(argc, argv);
        wilton::launcher::cli_options opts(launcher_argc, argv);
        
        // collect app args
        auto apprags = std::vector<std::string>();
        for (int i = launcher_argc + 1; i < argc; i++) {
            apprags.emplace_back(argv[i]);
            std::cout << "app: " << i << ": " << apprags.back() << std::endl;
        }

        // check invalid options
        if (!opts.parse_error.empty()) {
            std::cerr << "ERROR: " << opts.parse_error << std::endl;
            std::cerr << opts.usage() << std::endl;
            return 1;
        }

        // show help
        if (0 != opts.help) {
            std::cout << opts.usage() << std::endl;
            poptPrintHelp(opts.ctx, stdout, 0);
            return 0;
        }                
        
        // check startup script
        auto idxfile_or_dir = opts.indexjs;
        auto indexpath = sl::tinydir::path(idxfile_or_dir);
        if (!indexpath.exists()) {
            std::cerr << "ERROR: specified script file not found: [" + idxfile_or_dir + "]" << std::endl;
            return 1;
        }
        if (indexpath.is_directory() && '/' != idxfile_or_dir.at(idxfile_or_dir.length() -1)) {
            idxfile_or_dir.push_back('/');
        }
        
        auto exedir = find_exedir();
        
        // check modules dir
        auto moddir = !opts.modules_dir.empty() ? opts.modules_dir : exedir + "modules";
        auto modpath = sl::tinydir::path(moddir);
        if (!(modpath.exists() && modpath.is_directory())) {
            std::cerr << "ERROR: specified modules directory not found: [" + moddir + "]" << std::endl;
            return 1;
        }       
        
        // get index module
        auto startmod = find_startup_module(opts.startup_module_name, idxfile_or_dir);
        auto startmodpath = find_statup_module_path(idxfile_or_dir);

        // wilton init
        auto config = sl::json::dumps({
            {"defaultScriptEngine", "duktape"},
            {"requireJsConfig", {
                    {"waitSeconds", 0},
                    {"enforceDefine", true},
                    {"nodeIdCompat", true},
                    {"baseUrl", moddir},
                    {"paths", {
                        { startmod, "file://" + startmodpath }
                    }}
                }
            }
        });
        
//        std::cerr << config << std::endl;
        
        auto err_init = wiltoncall_init(config.c_str(), static_cast<int> (config.length()));
        if (nullptr != err_init) {
            std::cerr << "ERROR: " << err_init << std::endl;
            wilton_free(err_init);
            return 1;
        }

        // index.js input
        auto input = sl::json::dumps({
            {"module", startmod},
            {"func", "main"},
            {"args", [&apprags] {
                    auto res = std::vector<sl::json::value>();
                    for (auto& st : apprags) {
                        res.emplace_back(st);
                    }
                    return res;
                }()}
        });

        // init signals/ctrl+c to allow their use fron js
        sl::utils::initialize_signals();
        
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
}

