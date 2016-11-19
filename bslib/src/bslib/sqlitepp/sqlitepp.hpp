#pragma once

#include "bslib/sqlitepp/exceptions.hpp"
#include "bslib/sqlitepp/handles.hpp"
#include "bslib/sqlitepp/ScopedTransaction.hpp"
#include "bslib/sqlitepp/ScopedStatementReset.hpp"

#include <sqlite3.h>

namespace af {
namespace bslib {
namespace sqlitepp {

/**
 * Opens a database or throws an exception. This is equivalent to calling sqlite3_open_v2 and checking the return code.
 * \param db The database connection (UTF-8)
 * \param ppDb the SQLite db handle
 * \param flags Flags
 * \throws std::runtime_exception The database couldn't be opened
 */
void open_database_or_throw(const char* filename, sqlite3** ppDb, int flags);

/**
 * Prepares a statement or throws an exception. This is equivalent to calling sqlite3_prepare_v2 and checking the return code.
 * \param db The database connection
 * \param sql The statement SQL
 * \param statement The statement which must be free'd with sqlite3_finalize() when finished
 * \throws PrepareStatementFailedException The statement couldn't be prepared.
 */
void prepare_or_throw(sqlite3* db, const char* sql, sqlite3_stmt** statement);

/**
 * Executes the given SQL or throws an exception. This is equivalent to calling sqlite3_exec and checking the return code.
 * \param db The database connection
 * \param sql The statement SQL
 * \throws ExecuteFailedException The statement couldn't be prepared.
 */
void exec_or_throw(sqlite3* db, const char* sql);

/**
 * Binds a text parameter by name (e.g. :Foo)
 * \param statement The statement to bind the parameter to
 * \param name The name of the parameter
 * \param text The value of the parameter
 * \throws BindParameterFailedException The parameter couldn't be bound
 */
void BindByParameterNameText(sqlite3_stmt* statement, const std::string& name, const std::string& text);

/**
 * Binds an int64 parameter by name (e.g. :Foo)
 * \param statement The statement to bind the parameter to
 * \param name The name of the parameter
 * \param value The value of the parameter
 * \throws BindParameterFailedException The parameter couldn't be bound
 */
void BindByParameterNameInt64(sqlite3_stmt* statement, const std::string& name, int64_t value);

/**
 * Binds an int32 parameter by name (e.g. :Foo)
 * \param statement The statement to bind the parameter to
 * \param name The name of the parameter
 * \param value The value of the parameter
 * \throws BindParameterFailedException The parameter couldn't be bound
 */
void BindByParameterNameInt32(sqlite3_stmt* statement, const std::string& name, int32_t value);

/**
 * Binds a blob parameter by name (e.g. :Foo)
 * \param statement The statement to bind the parameter to
 * \param name The name of the parameter
 * \param blob Pointer to blob contents
 * \param size Size of the blob
 * \throws BindParameterFailedException The parameter couldn't be bound
 */
void BindByParameterNameBlob(sqlite3_stmt* statement, const std::string& name, const uint8_t* start, size_t size);

/**
 * Binds a null parameter by name (e.g. :Foo)
 * \param statement The statement to bind the parameter to
 * \param name The name of the parameter
 * \throws BindParameterFailedException The parameter couldn't be bound
 */
void BindByParameterNameNull(sqlite3_stmt* statement, const std::string& name);

}
}
}
