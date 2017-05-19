/* 
 * File:   wiltoncall_db.cpp
 * Author: alex
 *
 * Created on January 10, 2017, 6:14 PM
 */

#include "call/wiltoncall_internal.hpp"

#include "wilton/wilton.h"

namespace wilton {
namespace db {

namespace { //anonymous

common::handle_registry<wilton_DBConnection>& static_conn_registry() {
    static common::handle_registry<wilton_DBConnection> registry;
    return registry;
}

common::handle_registry<wilton_DBTransaction>& static_tran_registry() {
    static common::handle_registry<wilton_DBTransaction> registry;
    return registry;
}

} // namespace

std::string db_connection_open(const std::string& data) {
    wilton_DBConnection* conn;
    char* err = wilton_DBConnection_open(std::addressof(conn), data.c_str(), static_cast<int>(data.length()));
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    int64_t handle = static_conn_registry().put(conn);
    return sl::json::value({
        { "connectionHandle", handle}
    }).dumps();
}

std::string db_connection_query(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    auto rsql = std::ref(sl::utils::empty_string());
    std::string params = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else if ("sql" == name) {
            rsql = fi.as_string_nonempty_or_throw(name);
        } else if ("params" == name) {
            params = fi.val().dumps();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'connectionHandle' not specified"));
    if (rsql.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'sql' not specified"));
    const std::string& sql = rsql.get();
    if (params.empty()) {
        params = "{}";
    }
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_DBConnection_query(conn, sql.c_str(), static_cast<int>(sql.length()),
            params.c_str(), static_cast<int>(params.length()),
            std::addressof(out), std::addressof(out_len));
    static_conn_registry().put(conn);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return common::wrap_wilton_output(out, out_len);
}

std::string db_connection_execute(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    auto rsql = std::ref(sl::utils::empty_string());
    std::string params = sl::utils::empty_string();
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else if ("sql" == name) {
            rsql = fi.as_string_nonempty_or_throw(name);
        } else if ("params" == name) {
            params = fi.val().dumps();
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'connectionHandle' not specified"));
    if (rsql.get().empty()) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'sql' not specified"));
    const std::string& sql = rsql.get();
    if (params.empty()) {
        params = "{}";
    }
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified"));
    // call wilton
    char* err = wilton_DBConnection_execute(conn, sql.c_str(), static_cast<int>(sql.length()),
            params.c_str(), static_cast<int>(params.length()));
    static_conn_registry().put(conn);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return "{}";
}

std::string db_connection_close(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'connectionHandle' not specified"));
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified"));
    // call wilton
    char* err = wilton_DBConnection_close(conn);
    if (nullptr != err) {
        static_conn_registry().put(conn);
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

std::string db_transaction_start(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'connectionHandle' not specified"));
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified"));
    wilton_DBTransaction* tran;
    char* err = wilton_DBTransaction_start(conn, std::addressof(tran));
    static_conn_registry().put(conn);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err +
            "\ndb_transaction_start error for input data"));
    int64_t thandle = static_tran_registry().put(tran);
    return sl::json::value({
        { "transactionHandle", thandle}
    }).dumps();
}

std::string db_transaction_commit(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("transactionHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'transactionHandle' not specified"));
    // get handle
    wilton_DBTransaction* tran = static_tran_registry().remove(handle);
    if (nullptr == tran) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'transactionHandle' parameter specified"));
    char* err = wilton_DBTransaction_commit(tran);
    if (nullptr != err) {
        static_tran_registry().put(tran);
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

std::string db_transaction_rollback(const std::string& data) {
    // json parse
    sl::json::value json = sl::json::loads(data);
    int64_t handle = -1;
    for (const sl::json::field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("transactionHandle" == name) {
            handle = fi.as_int64_or_throw(name);
        } else {
            throw common::wilton_internal_exception(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::wilton_internal_exception(TRACEMSG(
            "Required parameter 'transactionHandle' not specified"));
    // get handle
    wilton_DBTransaction* tran = static_tran_registry().remove(handle);
    if (nullptr == tran) throw common::wilton_internal_exception(TRACEMSG(
            "Invalid 'transactionHandle' parameter specified"));
    char* err = wilton_DBTransaction_rollback(tran);
    if (nullptr != err) {
        static_tran_registry().put(tran);
        common::throw_wilton_error(err, TRACEMSG(err));
    }
    return "{}";
}

} // namespace
}
