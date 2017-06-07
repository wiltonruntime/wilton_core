/* 
 * File:   winservice.cpp
 * Author: alex
 *
 * Created on June 6, 2017, 6:31 PM
 */

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <popt.h>

#include "staticlib/io.hpp"
#include "staticlib/json.hpp"
#include "staticlib/ranges.hpp"
#include "staticlib/tinydir.hpp"
#include "staticlib/utils.hpp"
#include "staticlib/winservice.hpp"

#include "wilton/wilton.h"
#include "wilton/wiltoncall.h"

#include "winservice_config.hpp"
#include "winservice_exception.hpp"
#include "winservice_options.hpp"

namespace { //anonymous

std::string current_exedir() {
    auto exepath = sl::utils::current_executable_path();
    auto exedir = sl::utils::strip_filename(exepath);
    std::replace(exedir.begin(), exedir.end(), '\\', '/');
    return exedir;
}

wilton::launcher::winservice_config load_config(const std::string& exedir, char* cpath) {
    auto path = [&exedir, cpath] {
        if (nullptr != cpath) {
            return std::string(cpath);
        }
        return exedir + "config.json";
    }();
    std::map<std::string, std::string> values = {{"appdir", exedir}};
    auto onerr = [](const std::string & err) {
        throw wilton::launcher::winservice_exception(TRACEMSG(err));
    };
    try {
        auto src = sl::io::replacer_source<sl::tinydir::file_source>(
                sl::tinydir::file_source(path), values, onerr, "${", "}");
        auto json = sl::json::load(src);
        return wilton::launcher::winservice_config(std::move(json));
    } catch (const std::exception& e) {
        throw wilton::launcher::winservice_exception(TRACEMSG(e.what() + 
                "\nError loading config file, path: [" + path + "]"));
    }
}

void init_wilton(const std::string& exedir) {
    auto config = sl::json::dumps({
        {"defaultScriptEngine", "duktape"},
        {"requireJsDirPath", exedir + "requirejs"},
        {"requireJsConfig", {
                {"waitSeconds", 0},
                {"enforceDefine", true},
                {"nodeIdCompat", true},
                {"baseUrl", exedir + "modules"}
            }}
    });
    auto err = wiltoncall_init(config.c_str(), static_cast<int> (config.length()));
    if (nullptr != err) {
        auto msg = TRACEMSG(err);
        wilton_free(err);
        throw wilton::launcher::winservice_exception(msg);
    }
}

void run_script(const std::string& func, const wilton::launcher::winservice_config& conf) {
    std::string in = sl::json::dumps({
        { "module", "index"},
        { "func", func},
        { "args", [&conf] {
                auto vec = std::vector<sl::json::value>();
                vec.emplace_back(conf.json_config.clone());
                return vec;
            } ()},
    });
    char* out = nullptr;
    int out_len;
    auto err = wiltoncall_runscript_duktape(in.c_str(), in.length(), std::addressof(out), std::addressof(out_len));
    if (nullptr != err) {
        auto msg = TRACEMSG(err);
        wilton_free(err);
        throw wilton::launcher::winservice_exception(msg);
    }
    if (nullptr != out) {
        auto res = std::string(out, static_cast<uint32_t> (out_len));
        wilton_free(out);
        if (out_len > 0) {
            std::cout << res << std::endl;
        }
    }
}

void install(const wilton::launcher::winservice_config& conf) {
    std::cout << "Installing service with config: [" + conf.to_json().dumps() + "]" << std::endl;
    if (!sl::utils::current_process_elevated()) {
        throw wilton::launcher::winservice_exception(TRACEMSG(
            "Service install error, must be run under elevated (Administrator) account"));
    }
    sl::utils::ensure_has_logon_as_service(conf.user);
    sl::winservice::install_service(conf.service_name, conf.display_name,
            ".\\" + conf.user, conf.password);
    sl::winservice::start_service(conf.service_name);
}

void uninstall(const wilton::launcher::winservice_config& conf) {
    std::cout << "Uninstalling service with config: [" + conf.to_json().dumps() + "]" << std::endl;
    if (!sl::utils::current_process_elevated()) {
        throw wilton::launcher::winservice_exception(TRACEMSG(
            "Service uninstall error, must be run under elevated (Administrator) account"));
    }
    sl::winservice::uninstall_service(conf.service_name);
}

void stop(const wilton::launcher::winservice_config& conf) {
    std::cout << "Stopping service, name: [" + conf.service_name + "]" << std::endl;
    sl::winservice::stop_service(conf.service_name);
}

void start_service_and_wait(const wilton::launcher::winservice_config& conf) {
    std::cout << "Starting service with config: [" + conf.to_json().dumps() + "]" << std::endl;
    auto cf = std::make_shared<wilton::launcher::winservice_config>(conf.clone());
    sl::winservice::start_service_and_wait(conf.service_name,
    [cf] {
        run_script("start", *cf);
    },
    [cf] {
        run_script("stop", *cf);
        std::cout << "Stopping service ..." << std::endl;
    },
    [](const std::string& msg) {
        std::cerr << msg << std::endl;
    });
    std::cout << "Service stopped" << std::endl;
}

} // namespace

int main(int argc, char** argv) {

    // parse command line
    wilton::launcher::winservice_options opts(argc, argv);

    // check invalid options
    if (!opts.parse_error.empty()) {
        std::cerr << "ERROR: " << opts.parse_error << std::endl;
        poptPrintUsage(opts.ctx, stderr, 0);
        return 1;
    }

    // show help
    if (opts.help) {
        poptPrintHelp(opts.ctx, stdout, 0);
        return 0;
    }
    
    try {
        std::string exedir = current_exedir();
        init_wilton(exedir);
        wilton::launcher::winservice_config conf = load_config(exedir, opts.config);

        if (opts.install) {
            install(conf);
        } else if (opts.uninstall) {
            uninstall(conf);
        } else if (opts.stop) {
            stop(conf);
        } else if (opts.direct) {
            run_script("start", conf);
        } else { // SCM call            
            start_service_and_wait(conf);
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}

