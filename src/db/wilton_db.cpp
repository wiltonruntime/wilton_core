/* 
 * File:   wilton_db.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 8:22 PM
 */

#include "wilton/wilton.h"

#include "staticlib/config.hpp"
#include "staticlib/orm.hpp"
#include "staticlib/serialization.hpp"
#include "staticlib/utils.hpp"

namespace { // anonymous

namespace sc = staticlib::config;
namespace so = staticlib::orm;
namespace ss = staticlib::serialization;
namespace su = staticlib::utils;

}

struct wilton_DBConnection {
private:
    so::Connection conn;

public:

    wilton_DBConnection(so::Connection&& conn) :
    conn(std::move(conn)) { }

    so::Connection& impl() {
        return conn;
    }
};

struct wilton_DBTransaction {
private:
    so::Transaction tran;

public:

    wilton_DBTransaction(so::Transaction&& tran) :
    tran(std::move(tran)) { }

    so::Transaction& impl() {
        return tran;
    }
};

char* wilton_DBConnection_open(
        wilton_DBConnection** conn_out,
        const char* conn_url,
        int conn_url_len) /* noexcept */ {
    if (nullptr == conn_out) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'conn_out' parameter specified"));
    if (nullptr == conn_url) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'conn_url' parameter specified"));
    if (conn_url_len <= 0 ||
            static_cast<uint32_t> (conn_url_len) > std::numeric_limits<uint16_t>::max()) return su::alloc_copy(TRACEMSG(std::string() +
            "Invalid 'conn_url_len' parameter specified: [" + sc::to_string(conn_url_len) + "]"));
    try {
        uint16_t conn_url_len_u16 = static_cast<uint16_t> (conn_url_len);
        std::string conn_url_str{conn_url, conn_url_len_u16};
        so::Connection conn{conn_url_str};
        wilton_DBConnection* conn_ptr = new wilton_DBConnection{std::move(conn)};
        *conn_out = conn_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(std::string() + e.what() + "\nException raised"));
    }
}


