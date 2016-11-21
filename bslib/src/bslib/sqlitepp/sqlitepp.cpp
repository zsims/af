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

void exec_or_throw(sqlite3* db, const char* sql)
{
	const auto executeResult = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
	if (executeResult != SQLITE_OK)
	{
		throw ExecuteFailedException(executeResult);
	}
}

void BindByParameterNameText(sqlite3_stmt* statement, const std::string& name, const std::string& value)
{
	const auto index = sqlite3_bind_parameter_index(statement, name.c_str());
	const auto bindResult = sqlite3_bind_text(statement, index, value.c_str(), -1, 0);
	if (bindResult != SQLITE_OK)
	{
		throw BindParameterFailedException(name, bindResult);
	}
}

void BindByParameterNameInt64(sqlite3_stmt* statement, const std::string& name, int64_t value)
{
	const auto index = sqlite3_bind_parameter_index(statement, name.c_str());
	const auto bindResult = sqlite3_bind_int64(statement, index, value);
	if (bindResult != SQLITE_OK)
	{
		throw BindParameterFailedException(name, bindResult);
	}
}

void BindByParameterNameInt32(sqlite3_stmt* statement, const std::string& name, int32_t value)
{
	const auto index = sqlite3_bind_parameter_index(statement, name.c_str());
	const auto bindResult = sqlite3_bind_int(statement, index, value);
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

void BindByParameterNameNull(sqlite3_stmt* statement, const std::string& name)
{
	const auto index = sqlite3_bind_parameter_index(statement, name.c_str());
	const auto bindResult = sqlite3_bind_null(statement, index);
	if (bindResult != SQLITE_OK)
	{
		throw BindParameterFailedException(name, bindResult);
	}
}

}
}
}
