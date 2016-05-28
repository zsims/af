#pragma once

#include <boost/core/noncopyable.hpp>
#include <sqlite3.h>

namespace af {
namespace ffs {
namespace sqlitepp {

/**
 * Resets a statement using sqlite3_reset, meaning the statement can be re-used.
 * Note that bindings are also reset.
 */
class ScopedStatementReset : private boost::noncopyable
{
public:
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