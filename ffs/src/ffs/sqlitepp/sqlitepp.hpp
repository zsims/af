#pragma once

#include "ffs/sqlitepp/exceptions.hpp"
#include "ffs/sqlitepp/handles.hpp"
#include "ffs/sqlitepp/ScopedTransaction.hpp"
#include "ffs/sqlitepp/ScopedStatementReset.hpp"

#include <sqlite3.h>

namespace af {
namespace ffs {
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

}
}
}
