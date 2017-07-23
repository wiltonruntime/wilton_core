/* 
 * File:   winservice_options.hpp
 * Author: alex
 *
 * Created on June 7, 2017, 8:36 AM
 */

#ifndef WILTON_WINSERVICE_OPTIONS_HPP
#define	WILTON_WINSERVICE_OPTIONS_HPP

#include <algorithm>
#include <string>
#include <vector>

#include <popt.h>

namespace wilton {
namespace launcher {

class winservice_options {
    std::vector<struct poptOption> table;

public:
    poptContext ctx = nullptr;
    std::string parse_error;
    std::vector<std::string> args;

    // public options list
    char* config_ptr = nullptr;
    std::string config;
    int install = 0;
    int uninstall = 0;
    int stop = 0;
    int direct = 0;
    int help = 0;

    winservice_options(int argc, char** argv) :
    table({
        { "config", 'c', POPT_ARG_STRING, std::addressof(config_ptr), static_cast<int> ('c'), "Path to config file", nullptr},
        { "install", 'i', POPT_ARG_NONE, std::addressof(install), static_cast<int> ('i'), "Install this executable as Windows Service", nullptr},
        { "uninstall", 'u', POPT_ARG_NONE, std::addressof(uninstall), static_cast<int> ('u'), "Uninstall Windows Service (must be stopped)", nullptr},
        { "stop", 's', POPT_ARG_NONE, std::addressof(stop), static_cast<int> ('s'), "Stop Windows Service", nullptr},
        { "direct", 'd', POPT_ARG_NONE, std::addressof(direct), static_cast<int> ('d'), "Run target script directly", nullptr},
        { "help", 'h', POPT_ARG_NONE, std::addressof(help), static_cast<int> ('h'), "Show this help message", nullptr},
        { nullptr, 0, 0, nullptr, 0, nullptr, nullptr}
    }) {

        { // create context
            ctx = poptGetContext(nullptr, argc, const_cast<const char**> (argv), table.data(), POPT_CONTEXT_NO_EXEC);
            if (!ctx) {
                parse_error.append("'poptGetContext' error");
                return;
            }
        }

        { // parse options
            int val;
            while ((val = poptGetNextOpt(ctx)) >= 0);
            if (val < -1) {
                parse_error.append(poptStrerror(val));
                parse_error.append(": ");
                parse_error.append(poptBadOption(ctx, POPT_BADOPTION_NOALIAS));
                return;
            }
        }

        { // collect arguments
            const char* ar;
            while (nullptr != (ar = poptGetArg(ctx))) {
                args.push_back(std::string(ar));
            }
        }

        { // check unneeded args
            if (args.size() > 0) {
                parse_error.append("invalid arguments specified:");
                for (auto& st : args) {
                    parse_error.append(" ");
                    parse_error.append(st);
                }
            }
        }
        
        // set vars
        config = nullptr != config_ptr ? std::string(config_ptr) : "";
        std::replace(config.begin(), config.end(), '\\', '/');
    }

    ~winservice_options() {
        poptFreeContext(ctx);
    }

    winservice_options(const winservice_options& other) = delete;

    winservice_options& operator=(const winservice_options& other) = delete;

};



} // namespace
}

#endif	/* WILTON_WINSERVICE_OPTIONS_HPP */

