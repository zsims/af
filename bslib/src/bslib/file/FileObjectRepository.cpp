#include "bslib/file/FileObjectRepository.hpp"

#include "bslib/file/FileObject.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

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
	GetObject_ColumnIndex_Address = 0,
	GetObject_ColumnIndex_FullPath,
	GetObject_ColumnIndex_ContentBlobAddress,
	GetObject_ColumnIndex_ParentAddress
};
}

FileObjectRepository::FileObjectRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	// Prepare statements so they're good to go
	sqlitepp::prepare_or_throw(_db, "INSERT INTO FileObject (Address, FullPath, ContentBlobAddress, ParentAddress) VALUES (:Address, :FullPath, :ContentBlobAddress, :ParentAddress)", _insertObjectStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Address, FullPath, ContentBlobAddress, ParentAddress FROM FileObject
		WHERE Address = :Address
	)", _getObjectStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Address, FullPath, ContentBlobAddress, ParentAddress FROM FileObject
	)", _getAllObjectsStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Address, FullPath, ContentBlobAddress, ParentAddress FROM FileObject WHERE ParentAddress = :ParentAddress
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

std::vector<std::shared_ptr<FileObject>> FileObjectRepository::GetAllObjectsByParentAddress(const ObjectAddress& parentAddress) const
{
	std::vector<std::shared_ptr<FileObject>> result;
	sqlitepp::ScopedStatementReset reset(_getAllObjectsByParentStatement);
	const auto& binaryAddress = parentAddress.ToBinary();
	sqlitepp::BindByParameterNameBlob(_getAllObjectsByParentStatement, ":ParentAddress", &binaryAddress[0], binaryAddress.size());

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getAllObjectsByParentStatement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToObject(_getAllObjectsByParentStatement));
	}

	return result;
}

void FileObjectRepository::AddObject(const FileObject& info)
{
	// binary address, note this has to be kept in scope until SQLite has finished as we've opted not to make a copy
	const auto binaryAddress = info.address.ToBinary();
	const auto fullPath = info.fullPath;
	sqlitepp::ScopedStatementReset reset(_insertObjectStatement);
	
	sqlitepp::BindByParameterNameBlob(_insertObjectStatement, ":Address", &binaryAddress[0], binaryAddress.size());
	sqlitepp::BindByParameterNameText(_insertObjectStatement, ":FullPath", fullPath);

	if (info.contentBlobAddress)
	{
		const auto binaryContentAddress = info.contentBlobAddress.value().ToBinary();
		sqlitepp::BindByParameterNameBlob(_insertObjectStatement, ":ContentBlobAddress", &binaryContentAddress[0], binaryContentAddress.size());
	}
	else
	{
		sqlitepp::BindByParameterNameNull(_insertObjectStatement, ":ContentBlobAddress");
	}

	if (info.parentAddress)
	{
		const auto binaryParentAddress = info.parentAddress.value().ToBinary();
		sqlitepp::BindByParameterNameBlob(_insertObjectStatement, ":ParentAddress", &binaryParentAddress[0], binaryParentAddress.size());
	}
	else
	{
		sqlitepp::BindByParameterNameNull(_insertObjectStatement, ":ParentAddress");
	}

	const auto stepResult = sqlite3_step(_insertObjectStatement);
	if (stepResult != SQLITE_DONE)
	{
		throw AddObjectFailedException((boost::format("Failed to execute statement for insert object %1%. SQLite error %2%") % info.address.ToString() % stepResult).str());
	}
}

FileObject FileObjectRepository::GetObject(const ObjectAddress& address) const
{
	const auto& info = FindObject(address);
	if(!info)
	{
		throw ObjectNotFoundException((boost::format("Object with address %1% not found.") % address.ToString()).str());
	}

	return info.value();
}

boost::optional<FileObject> FileObjectRepository::FindObject(const ObjectAddress& address) const
{
	const auto binaryAddress = address.ToBinary();
	sqlitepp::ScopedStatementReset reset(_getObjectStatement);
	sqlitepp::BindByParameterNameBlob(_getObjectStatement, ":Address", &binaryAddress[0], binaryAddress.size());

	auto stepResult = sqlite3_step(_getObjectStatement);
	if (stepResult != SQLITE_ROW)
	{
		return boost::none;
	}

	return *MapRowToObject(_getObjectStatement);
}

std::shared_ptr<FileObject> FileObjectRepository::MapRowToObject(const sqlitepp::ScopedStatement& statement) const
{
	const auto objectAddressBytes = sqlite3_column_blob(statement, GetObject_ColumnIndex_Address);
	const auto objectAddressBytesCount = sqlite3_column_bytes(statement, GetObject_ColumnIndex_Address);
	const ObjectAddress objectAddress(objectAddressBytes, objectAddressBytesCount);

	const auto rawFullPath = sqlite3_column_text(statement, GetObject_ColumnIndex_FullPath);
	const auto fullPath = std::string(reinterpret_cast<const char*>(rawFullPath));
	
	const auto contentBlobAddressBytesCount = sqlite3_column_bytes(statement, GetObject_ColumnIndex_ContentBlobAddress);
	boost::optional<BlobAddress> contentBlobAddress = boost::none;
	if (contentBlobAddressBytesCount > 0)
	{
		const auto contentBlobAddressBytes = sqlite3_column_blob(statement, GetObject_ColumnIndex_ContentBlobAddress);
		contentBlobAddress = BlobAddress(contentBlobAddressBytes, contentBlobAddressBytesCount);
	}

	const auto parentAddressBytesCount = sqlite3_column_bytes(statement, GetObject_ColumnIndex_ParentAddress);
	boost::optional<ObjectAddress> parentAddress = boost::none;
	if (parentAddressBytesCount > 0)
	{
		const auto parentAddressBytes = sqlite3_column_blob(statement, GetObject_ColumnIndex_ParentAddress);
		parentAddress = ObjectAddress(parentAddressBytes, parentAddressBytesCount);
	}

	return std::make_shared<FileObject>(objectAddress, fullPath, contentBlobAddress, parentAddress);
}

}
}
}