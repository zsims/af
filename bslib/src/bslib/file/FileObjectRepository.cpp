#include "bslib/file/FileObjectRepository.hpp"

#include "bslib/Address.hpp"
#include "bslib/file/FileObject.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <sqlite3.h>

#include <memory>
#include <utility>

namespace af {
namespace bslib {
namespace file {

namespace {
enum GetObjectColumnIndex
{
	GetObject_ColumnIndex_Id = 0,
	GetObject_ColumnIndex_FullPath,
	GetObject_ColumnIndex_ContentBlobAddress,
	GetObject_ColumnIndex_ParentId
};
}

FileObjectRepository::FileObjectRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	// Prepare statements so they're good to go
	sqlitepp::prepare_or_throw(_db, "INSERT INTO FileObject (FullPath, ContentBlobAddress, ParentId) VALUES (:FullPath, :ContentBlobAddress, :ParentId)", _insertObjectStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Id, FullPath, ContentBlobAddress, ParentId FROM FileObject WHERE Id = :Id
	)", _getObjectStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Id , FullPath, ContentBlobAddress, ParentId FROM FileObject
	)", _getAllObjectsStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Id , FullPath, ContentBlobAddress, ParentId FROM FileObject WHERE ParentId = :ParentId
	)", _getAllObjectsByParentStatement);
}

std::vector<std::shared_ptr<FileObject>> FileObjectRepository::GetAllObjects() const
{
	std::vector<std::shared_ptr<FileObject>> result;
	sqlitepp::ScopedStatementReset reset(_getAllObjectsStatement);

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getAllObjectsStatement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToObject(_getAllObjectsStatement));
	}

	return result;
}

std::vector<std::shared_ptr<FileObject>> FileObjectRepository::GetAllObjectsByParentId(foid parentId) const
{
	std::vector<std::shared_ptr<FileObject>> result;
	sqlitepp::ScopedStatementReset reset(_getAllObjectsByParentStatement);
	sqlitepp::BindByParameterNameInt64(_getAllObjectsByParentStatement, ":ParentId", parentId);
	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getAllObjectsByParentStatement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToObject(_getAllObjectsByParentStatement));
	}

	return result;
}

foid FileObjectRepository::AddObject(
	const boost::filesystem::path& fullPath,
	const boost::optional<BlobAddress>& contentBlobAddress,
	const boost::optional<foid>& parentId)
{
	sqlitepp::ScopedStatementReset reset(_insertObjectStatement);
	const auto& rawPath = fullPath.string();
	sqlitepp::BindByParameterNameText(_insertObjectStatement, ":FullPath", rawPath);

	if (contentBlobAddress)
	{
		const auto binaryContentAddress = contentBlobAddress.value().ToBinary();
		sqlitepp::BindByParameterNameBlob(_insertObjectStatement, ":ContentBlobAddress", &binaryContentAddress[0], binaryContentAddress.size());
	}
	else
	{
		sqlitepp::BindByParameterNameNull(_insertObjectStatement, ":ContentBlobAddress");
	}

	if (parentId)
	{
		sqlitepp::BindByParameterNameInt64(_insertObjectStatement, ":ParentId", parentId.value());
	}
	else
	{
		sqlitepp::BindByParameterNameNull(_insertObjectStatement, ":ParentId");
	}

	const auto stepResult = sqlite3_step(_insertObjectStatement);
	if (stepResult != SQLITE_DONE)
	{
		throw AddObjectFailedException((boost::format("Failed to execute statement for insert object. SQLite error %1%") % stepResult).str());
	}

	return sqlite3_last_insert_rowid(_db);
}

FileObject FileObjectRepository::AddGetObject(
	const boost::filesystem::path& fullPath,
	const boost::optional<BlobAddress>& contentBlobAddress,
	const boost::optional<foid>& parentId)
{
	const auto id = AddObject(fullPath, contentBlobAddress, parentId);
	return FileObject(id, fullPath, contentBlobAddress, parentId);
}

FileObject FileObjectRepository::GetObject(foid id) const
{
	const auto& info = FindObject(id);
	if(!info)
	{
		throw ObjectNotFoundException((boost::format("Object with id %1% not found.") % id).str());
	}

	return info.value();
}

boost::optional<FileObject> FileObjectRepository::FindObject(foid id) const
{
	sqlitepp::ScopedStatementReset reset(_getObjectStatement);
	sqlitepp::BindByParameterNameInt64(_getObjectStatement, ":Id", id);

	auto stepResult = sqlite3_step(_getObjectStatement);
	if (stepResult != SQLITE_ROW)
	{
		return boost::none;
	}

	return *MapRowToObject(_getObjectStatement);
}

std::shared_ptr<FileObject> FileObjectRepository::MapRowToObject(const sqlitepp::ScopedStatement& statement) const
{
	const foid id = sqlite3_column_int64(statement, GetObject_ColumnIndex_Id);

	const auto rawFullPath = sqlite3_column_text(statement, GetObject_ColumnIndex_FullPath);
	const auto fullPath = std::string(reinterpret_cast<const char*>(rawFullPath));
	
	const auto contentBlobAddressBytesCount = sqlite3_column_bytes(statement, GetObject_ColumnIndex_ContentBlobAddress);
	boost::optional<BlobAddress> contentBlobAddress = boost::none;
	if (contentBlobAddressBytesCount > 0)
	{
		const auto contentBlobAddressBytes = sqlite3_column_blob(statement, GetObject_ColumnIndex_ContentBlobAddress);
		contentBlobAddress = BlobAddress(contentBlobAddressBytes, contentBlobAddressBytesCount);
	}

	boost::optional<foid> parentId = boost::none;
	if (sqlite3_column_type(statement, GetObject_ColumnIndex_ParentId) != SQLITE_NULL)
	{
		parentId = sqlite3_column_int64(statement, GetObject_ColumnIndex_ParentId);
	}

	return std::make_shared<FileObject>(id, fullPath, contentBlobAddress, parentId);
}

}
}
}