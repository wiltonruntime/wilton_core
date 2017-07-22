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

std::string replace_appdir(const std::string& str, const std::string& exedir) {
    auto values = sl::json::dumps({
        { "exedir", exedir }
    });
    char* out = nullptr;
    int out_len = 0;
    auto err = wilton_render_mustache(str.c_str(), static_cast<int>(str.length()),
            values.c_str(), values.length(), std::addressof(out), std::addressof(out_len));
    if (nullptr != err) {
        auto msg = TRACEMSG(err);
        wilton_free(err);
        throw wilton::launcher::winservice_exception(msg);
    }
    auto out_str = std::string(out, out_len);
    wilton_free(out);
    return out_str;
}

wilton::launcher::winservice_config load_config(const std::string& exedir, const std::string& cpath) {
    try {
        // get path
        auto path = !cpath.empty() ? cpath : exedir + "config.json";

        // load file
        auto src = sl::tinydir::file_source(path);
        auto conf = sl::json::load(src);

        // substitute appdir in wilton paths
        auto& wconf = conf.getattr_or_throw("wiltonConfig", "wiltonConfig");
        auto& rjsconf = wconf.getattr_or_throw("requireJsConfig", "wiltonConfig.requireJsConfig");
        auto& base_url = rjsconf.getattr_or_throw("baseUrl", "wiltonConfig.requireJsConfig.baseUrl");
        auto base_url_replaced = replace_appdir(base_url.as_string_nonempty_or_throw("wiltonConfig.requireJsConfig.baseUrl"), exedir);
        base_url.set_string(base_url_replaced);
        auto& paths = rjsconf.getattr_or_throw("paths", "wiltonConfig.requireJsConfig.paths");
        for (auto& pa : paths.as_object_or_throw("wiltonConfig.requireJsConfig.paths")) {
            auto pa_replaced = replace_appdir(pa.as_string_nonempty_or_throw("wiltonConfig.requireJsConfig.paths._"), exedir);
            pa.val().set_string(pa_replaced);
        }
        
        // return partially parsed config
        return wilton::launcher::winservice_config(std::move(conf));
    } catch (const std::exception& e) {
        throw wilton::launcher::winservice_exception(TRACEMSG(e.what() + 
                "\nError loading config file, path: [" + path + "]"));
    }
}

void init_wilton(wilton::launcher::winservice_config& sconf) {
    auto& wconf = sconf.json_config.getattr_or_throw("wiltonConfig", "wiltonConfig");
    auto err = wiltoncall_init(wconf.c_str(), static_cast<int> (wconf.length()));
    if (nullptr != err) {
        auto msg = TRACEMSG(err);
        wilton_free(err);
        throw wilton::launcher::winservice_exception(msg);
    }
}

void run_script(const wilton::launcher::winservice_config& conf) {
    std::string in = sl::json::dumps({
        { "module", conf.startup_module},
        { "func", "main"}
    });
    char* out = nullptr;
    int out_len = 0;
    auto err = wiltoncall_runscript_duktape(in.c_str(), static_cast<int> (in.length()), std::addressof(out), std::addressof(out_len));
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
        run_script(*cf);
    },
    [] {
        wilton_thread_fire_signal();
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
        wilton::launcher::winservice_config conf = load_config(exedir, opts.config);
        init_wilton(conf);

        if (opts.install) {
            install(conf);
        } else if (opts.uninstall) {
            uninstall(conf);
        } else if (opts.stop) {
            stop(conf);
        } else { // SCM call            
            start_service_and_wait(conf);
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }
}

