#include "bslib/file/FileRefRepository.hpp"

#include "bslib/file/FileRef.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <sqlite3.h>

#include <memory>
#include <utility>

namespace af {
namespace bslib {
namespace file {

namespace {
enum GetRefColumnIndex
{
	GetRef_ColumnIndex_FullPath = 0,
	GetRef_ColumnIndex_FileObjectId
};
}

FileRefRepository::FileRefRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	sqlitepp::prepare_or_throw(_db, "INSERT OR REPLACE INTO FileRef (FullPath, FileObjectId) VALUES (:FullPath, :FileObjectId)", _insertRefStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT FullPath, FileObjectId FROM FileRef
		WHERE FullPath = :FullPath
	)", _getRefStatement);
}

void FileRefRepository::SetReference(const FileRef& reference)
{
	const auto& fullPath = reference.fullPath;
	sqlitepp::ScopedStatementReset reset(_insertRefStatement);
	
	sqlitepp::BindByParameterNameText(_insertRefStatement, ":FullPath", fullPath);
	sqlitepp::BindByParameterNameInt64(_insertRefStatement, ":FileObjectId", reference.fileObjectId);

	const auto stepResult = sqlite3_step(_insertRefStatement);
	if (stepResult != SQLITE_DONE)
	{
		throw AddRefFailedException(reference.fullPath, stepResult);
	}
}

FileRef FileRefRepository::GetReference(const std::string& fullPath) const
{
	const auto ref = FindReference(fullPath);
	if(!ref)
	{
		throw RefNotFoundException(fullPath);
	}
	return ref.value();
}

boost::optional<FileRef> FileRefRepository::FindReference(const std::string& fullPath) const
{
	sqlitepp::ScopedStatementReset reset(_getRefStatement);
	sqlitepp::BindByParameterNameText(_getRefStatement, ":FullPath", fullPath);

	auto stepResult = sqlite3_step(_getRefStatement);
	if (stepResult != SQLITE_ROW)
	{
		return boost::none;
	}

	const auto fileId = sqlite3_column_int64(_getRefStatement, GetRef_ColumnIndex_FileObjectId);
	return FileRef(fullPath, fileId);
}

}
}
}