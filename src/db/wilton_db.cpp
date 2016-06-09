/* 
 * File:   wilton_db.cpp
 * Author: alex
 * 
 * Created on June 4, 2016, 8:22 PM
 */

#include "wilton/wilton.h"

#include <string>
#include <vector>

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

// todo: json copy
char* wilton_DBConnection_query(
        wilton_DBConnection* conn,
        const char* sql_text,
        int sql_text_len,
        const char* params_json,
        int params_json_len,
        char** result_set_out,
        int* result_set_len_out) {
    if (nullptr == conn) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'conn' parameter specified"));
    if (nullptr == sql_text) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'sql_text' parameter specified"));
    if (sql_text_len < 0 ||
            static_cast<uint64_t> (sql_text_len) > std::numeric_limits<uint32_t>::max()) return su::alloc_copy(TRACEMSG(std::string() +
            "Invalid 'sql_text_len' parameter specified: [" + sc::to_string(sql_text_len) + "]"));
    if (nullptr == params_json) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'params_json' parameter specified"));
    if (params_json_len < 0 ||
            static_cast<uint64_t> (params_json_len) > std::numeric_limits<uint32_t>::max()) return su::alloc_copy(TRACEMSG(std::string() +
            "Invalid 'params_json_len' parameter specified: [" + sc::to_string(params_json_len) + "]"));
    if (nullptr == result_set_out) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'result_set_out' parameter specified"));
    if (nullptr == result_set_len_out) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'result_set_len_out' parameter specified"));
    try {
        uint32_t sql_text_len_u32 = static_cast<uint32_t> (sql_text_len);
        std::string sql_text_str{sql_text, sql_text_len_u32};
        ss::JsonValue json{};
        if (params_json_len > 0) {
            uint32_t params_json_len_u32 = static_cast<uint32_t> (params_json_len);
            std::string params_json_str{params_json, params_json_len_u32};        
            json = ss::load_json_from_string(params_json_str);
        }
        std::vector<ss::JsonValue> rs = conn->impl().query(sql_text_str, json);
        ss::JsonValue rs_json{std::move(rs)};
        std::string rs_str = ss::dump_json_to_string(rs_json);
        *result_set_out = su::alloc_copy(rs_str);
        *result_set_len_out = rs_str.size();
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(std::string() + e.what() + "\nException raised"));
    }
}

char* wilton_DBConnection_execute(
        wilton_DBConnection* conn,
        const char* sql_text,
        int sql_text_len,
        const char* params_json,
        int params_json_len) {
    if (nullptr == conn) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'conn' parameter specified"));
    if (nullptr == sql_text) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'sql_text' parameter specified"));
    if (sql_text_len < 0 ||
            static_cast<uint64_t> (sql_text_len) > std::numeric_limits<uint32_t>::max()) return su::alloc_copy(TRACEMSG(std::string() +
            "Invalid 'sql_text_len' parameter specified: [" + sc::to_string(sql_text_len) + "]"));
    if (nullptr == params_json) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'params_json' parameter specified"));
    if (params_json_len < 0 ||
            static_cast<uint64_t> (params_json_len) > std::numeric_limits<uint32_t>::max()) return su::alloc_copy(TRACEMSG(std::string() +
            "Invalid 'params_json_len' parameter specified: [" + sc::to_string(params_json_len) + "]"));
    try {
        uint32_t sql_text_len_u32 = static_cast<uint32_t> (sql_text_len);
        std::string sql_text_str{sql_text, sql_text_len_u32};
        ss::JsonValue json{};
        if (params_json_len > 0) {
            uint32_t params_json_len_u32 = static_cast<uint32_t> (params_json_len);
            std::string params_json_str{params_json, params_json_len_u32};
            json = ss::load_json_from_string(params_json_str);
        }
        conn->impl().execute(sql_text_str, json);
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(std::string() + e.what() + "\nException raised"));
    }    
}

char* wilton_DBConnection_close(
        wilton_DBConnection* conn) {
    if (nullptr == conn) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'conn' parameter specified"));
    try {
        delete conn;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(std::string() + e.what() + "\nException raised"));
    } 
}

char* wilton_DBTransaction_start(
        wilton_DBConnection* conn,
        wilton_DBTransaction** tran_out) {
    if (nullptr == conn) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'conn' parameter specified"));
    if (nullptr == tran_out) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'tran_out' parameter specified"));
    try {
        so::Transaction tran = conn->impl().start_transaction();
        wilton_DBTransaction* tran_ptr = new wilton_DBTransaction(std::move(tran));
        *tran_out = tran_ptr;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(std::string() + e.what() + "\nException raised"));
    }
}

WILTON_EXPORT char* wilton_DBTransaction_commit(
        wilton_DBTransaction* tran) {
    if (nullptr == tran) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'tran' parameter specified"));
    try {
        tran->impl().commit();
        delete tran;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(std::string() + e.what() + "\nException raised"));
    }
}

char* wilton_DBTransaction_rollback(
        wilton_DBTransaction* tran) {
    if (nullptr == tran) return su::alloc_copy(TRACEMSG(std::string() +
            "Null 'tran' parameter specified"));
    try {
        delete tran;
        return nullptr;
    } catch (const std::exception& e) {
        return su::alloc_copy(TRACEMSG(std::string() + e.what() + "\nException raised"));
    }
}
