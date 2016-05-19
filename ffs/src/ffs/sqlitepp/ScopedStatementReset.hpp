#pragma once

#include <sqlite3.h>

namespace af {
namespace ffs {
namespace sqlitepp {

/**
 * Resets a statement using sqlite3_reset, meaning the statement can be re-used.
 * Note that bindings are also reset.
 */
class ScopedStatementReset
{
public:
	ScopedStatementReset(const ScopedStatementReset& that) = delete;
	ScopedStatementReset& operator=(const ScopedStatementReset&) = delete;

	explicit ScopedStatementReset(sqlite3_stmt* statement)
		: _statement(statement)
	{
	}

	~ScopedStatementReset()
	{
		sqlite3_reset(_statement);
		sqlite3_clear_bindings(_statement);
	}
private:
	sqlite3_stmt* _statement;
};

}
}
}