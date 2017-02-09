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

namespace ss = staticlib::serialization;

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
    char* err = wilton_DBConnection_open(std::addressof(conn), data.c_str(), data.length());
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    int64_t handle = static_conn_registry().put(conn);
    return ss::dump_json_to_string({
        { "connectionHandle", handle}
    });
}

std::string db_connection_query(const std::string& data) {
    // json parse
    ss::json_value json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rsql = std::ref(common::empty_string());
    std::string params = common::empty_string();
    for (const ss::json_field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = common::get_json_int64(fi);
        } else if ("sql" == name) {
            rsql = common::get_json_string(fi);
        } else if ("params" == name) {
            params = ss::dump_json_to_string(fi.value());
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'connectionHandle' not specified"));
    if (rsql.get().empty()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'sql' not specified"));
    const std::string& sql = rsql.get();
    if (params.empty()) {
        params = "{}";
    }
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified"));
    // call wilton
    char* out;
    int out_len;
    char* err = wilton_DBConnection_query(conn, sql.c_str(), sql.length(),
            params.c_str(), params.length(), std::addressof(out), std::addressof(out_len));
    static_conn_registry().put(conn);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return common::wrap_wilton_output(out, out_len);
}

std::string db_connection_execute(const std::string& data) {
    // json parse
    ss::json_value json = ss::load_json_from_string(data);
    int64_t handle = -1;
    auto rsql = std::ref(common::empty_string());
    std::string params = common::empty_string();
    for (const ss::json_field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = common::get_json_int64(fi);
        } else if ("sql" == name) {
            rsql = common::get_json_string(fi);
        } else if ("params" == name) {
            params = ss::dump_json_to_string(fi.value());
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'connectionHandle' not specified"));
    if (rsql.get().empty()) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'sql' not specified"));
    const std::string& sql = rsql.get();
    if (params.empty()) {
        params = "{}";
    }
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified"));
    // call wilton
    char* err = wilton_DBConnection_execute(conn, sql.c_str(), sql.length(),
            params.c_str(), params.length());
    static_conn_registry().put(conn);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err));
    return "{}";
}

std::string db_connection_close(const std::string& data) {
    // json parse
    ss::json_value json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::json_field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'connectionHandle' not specified"));
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw common::WiltonInternalException(TRACEMSG(
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
    ss::json_value json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::json_field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("connectionHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'connectionHandle' not specified"));
    // get handle
    wilton_DBConnection* conn = static_conn_registry().remove(handle);
    if (nullptr == conn) throw common::WiltonInternalException(TRACEMSG(
            "Invalid 'connectionHandle' parameter specified"));
    wilton_DBTransaction* tran;
    char* err = wilton_DBTransaction_start(conn, std::addressof(tran));
    static_conn_registry().put(conn);
    if (nullptr != err) common::throw_wilton_error(err, TRACEMSG(err +
            "\ndb_transaction_start error for input data"));
    int64_t thandle = static_tran_registry().put(tran);
    return ss::dump_json_to_string({
        { "transactionHandle", thandle}
    });
}

std::string db_transaction_commit(const std::string& data) {
    // json parse
    ss::json_value json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::json_field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("transactionHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'transactionHandle' not specified"));
    // get handle
    wilton_DBTransaction* tran = static_tran_registry().remove(handle);
    if (nullptr == tran) throw common::WiltonInternalException(TRACEMSG(
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
    ss::json_value json = ss::load_json_from_string(data);
    int64_t handle = -1;
    for (const ss::json_field& fi : json.as_object()) {
        auto& name = fi.name();
        if ("transactionHandle" == name) {
            handle = common::get_json_int64(fi);
        } else {
            throw common::WiltonInternalException(TRACEMSG("Unknown data field: [" + name + "]"));
        }
    }
    if (-1 == handle) throw common::WiltonInternalException(TRACEMSG(
            "Required parameter 'transactionHandle' not specified"));
    // get handle
    wilton_DBTransaction* tran = static_tran_registry().remove(handle);
    if (nullptr == tran) throw common::WiltonInternalException(TRACEMSG(
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
