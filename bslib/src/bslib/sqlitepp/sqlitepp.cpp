#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/format.hpp>

#include <string>

namespace af {
namespace bslib {
namespace sqlitepp {
	
void open_database_or_throw(const char* filename, sqlite3** ppDb, int flags)
{
	const auto result = sqlite3_open_v2(filename, ppDb, flags, 0);
	if (result != SQLITE_OK)
	{
		throw OpenDatabaseFailedException(filename, result);
	}
}

void prepare_or_throw(sqlite3* db, const char* sql, sqlite3_stmt** statement)
{
	const auto prepareResult = sqlite3_prepare_v2(db, sql, -1, statement, 0);
	if (prepareResult != SQLITE_OK)
	{
		throw PrepareStatementFailedException(prepareResult);
	}
}

void BindByParameterNameText(sqlite3_stmt* statement, const std::string& name, const std::string& text)
{
	const auto index = sqlite3_bind_parameter_index(statement, name.c_str());
	const auto bindResult = sqlite3_bind_text(statement, index, text.c_str(), -1, 0);
	if (bindResult != SQLITE_OK)
	{
		throw BindParameterFailedException(name, bindResult);
	}
}

void BindByParameterNameBlob(sqlite3_stmt* statement, const std::string& name, const uint8_t* start, size_t size)
{
	const auto index = sqlite3_bind_parameter_index(statement, name.c_str());
	const auto bindResult = sqlite3_bind_blob(statement, index, start, static_cast<int>(size), 0);
	if (bindResult != SQLITE_OK)
	{
		throw BindParameterFailedException(name, bindResult);
	}
}

}
}
}
