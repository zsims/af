
#include "ffs/sqlitepp/sqlitepp.hpp"

namespace af {
namespace ffs {
namespace sqlitepp {

void prepare_or_throw(sqlite3* db, const char* sql, sqlite3_stmt** statement)
{
	const auto prepareResult = sqlite3_prepare_v2(db, sql, -1, statement, 0);
	if (prepareResult != SQLITE_OK)
	{
		throw PrepareStatementFailedException(prepareResult);
	}
}

}
}
}
