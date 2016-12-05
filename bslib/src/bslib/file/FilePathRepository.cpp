#include "bslib/file/FilePathRepository.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

namespace af {
namespace bslib {
namespace file {

namespace {
enum FilePathColumnIndex
{
	FilePath_ColumnIndex_Id = 0,
	FilePath_ColumnIndex_FullPath,
	FilePath_ColumnIndex_ParentId
};
}

FilePathRepository::FilePathRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
}

std::vector<std::pair<int64_t, fs::NativePath>> FilePathRepository::GetAllPaths() const
{
	const auto query = "SELECT Id, FullPath FROM FilePath";
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query, statement);
	std::vector<std::pair<int64_t, fs::NativePath>> result;
	auto stepResult = 0;
	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW)
	{
		const auto id = sqlite3_column_int64(statement, FilePath_ColumnIndex_Id);
		const auto rawFullPath = sqlite3_column_text(statement, FilePath_ColumnIndex_FullPath);
		const fs::NativePath fullPath(reinterpret_cast<const char*>(rawFullPath));
		result.push_back(std::make_pair(id, fullPath));
	}

	return result;
}

int64_t FilePathRepository::AddPath(const fs::NativePath& path, const boost::optional<int64_t>& parentId)
{
	const auto query = "INSERT INTO FilePath (FullPath, ParentId) VALUES(:FullPath, :ParentId)";
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query, statement);
	const auto& rawPath = path.ToString();
	sqlitepp::BindByParameterNameText(statement, ":FullPath", rawPath);

	if (parentId)
	{
		sqlitepp::BindByParameterNameInt64(statement, ":ParentId", parentId.value());
	}
	else
	{
		sqlitepp::BindByParameterNameNull(statement, ":ParentId");
	}

	const auto stepResult = sqlite3_step(statement);
	if (stepResult != SQLITE_DONE)
	{
		throw AddFilePathFailedException(stepResult);
	}

	return sqlite3_last_insert_rowid(_db);
}

boost::optional<int64_t> FilePathRepository::FindPath(const fs::NativePath& path) const
{
	const auto query = "SELECT Id FROM FilePath WHERE FullPath = :FullPath";
	sqlitepp::ScopedStatement statement;
	sqlitepp::prepare_or_throw(_db, query, statement);
	const auto& rawPath = path.ToString();
	sqlitepp::BindByParameterNameText(statement, ":FullPath", rawPath);
	const auto stepResult = sqlite3_step(statement);
	if(stepResult == SQLITE_ROW)
	{
		return sqlite3_column_int64(statement, FilePath_ColumnIndex_Id);
	}

	return boost::none;
}

}
}
}