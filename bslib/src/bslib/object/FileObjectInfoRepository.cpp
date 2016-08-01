#include "bslib/object/FileObjectInfoRepository.hpp"

#include "bslib/object/FileObjectInfo.hpp"
#include "bslib/object/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/format.hpp>
#include <sqlite3.h>

#include <memory>
#include <utility>

namespace af {
namespace bslib {
namespace object {

namespace {
enum GetObjectColumnIndex
{
	GetObject_ColumnIndex_Address = 0,
	GetObject_ColumnIndex_FullPath,
	GetObject_ColumnIndex_ContentBlobAddress,
};
}

FileObjectInfoRepository::FileObjectInfoRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	// Prepare statements so they're good to go
	sqlitepp::prepare_or_throw(_db, "INSERT INTO FileObject (Address, FullPath, ContentBlobAddress) VALUES (:Address, :FullPath, :ContentBlobAddress)", _insertObjectStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Address, FullPath, ContentBlobAddress FROM FileObject
		WHERE Address = :Address
	)", _getObjectStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Address, FullPath, ContentBlobAddress FROM FileObject
	)", _getAllObjectsStatement);
}

std::vector<std::shared_ptr<FileObjectInfo>> FileObjectInfoRepository::GetAllObjects() const
{
	std::vector<std::shared_ptr<FileObjectInfo>> result;
	sqlitepp::ScopedStatementReset reset(_getAllObjectsStatement);

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getAllObjectsStatement)) == SQLITE_ROW)
	{
		result.push_back(MapRowToObject(_getAllObjectsStatement));
	}

	return result;
}


void FileObjectInfoRepository::AddObject(const FileObjectInfo& info)
{
	// binary address, note this has to be kept in scope until SQLite has finished as we've opted not to make a copy
	const auto binaryAddress = info.address.ToBinary();
	const auto binaryContentAddress = info.contentBlobAddress.ToBinary();
	const auto fullPath = info.fullPath;
	sqlitepp::ScopedStatementReset reset(_insertObjectStatement);
	
	{
		const auto index = sqlite3_bind_parameter_index(_insertObjectStatement, ":Address");
		const auto bindResult = sqlite3_bind_blob(_insertObjectStatement, index, &binaryAddress[0], static_cast<int>(binaryAddress.size()), 0);
		if (bindResult != SQLITE_OK)
		{
			throw AddObjectFailedException((boost::format("Failed to bind object parameter address %1%. SQLite error %2%") % info.address.ToString() % bindResult).str());
		}
	}

	{
		const auto index = sqlite3_bind_parameter_index(_insertObjectStatement, ":ContentBlobAddress");
		const auto bindResult = sqlite3_bind_blob(_insertObjectStatement, index, &binaryContentAddress[0], static_cast<int>(binaryContentAddress.size()), 0);
		if (bindResult != SQLITE_OK)
		{
			throw AddObjectFailedException((boost::format("Failed to bind object parameter content blob address %1%. SQLite error %2%") % info.contentBlobAddress.ToString() % bindResult).str());
		}
	}

	{
		const auto index = sqlite3_bind_parameter_index(_insertObjectStatement, ":FullPath");
		const auto bindResult = sqlite3_bind_text(_insertObjectStatement, index, fullPath.c_str(), -1, 0);
		if (bindResult != SQLITE_OK)
		{
			throw AddObjectFailedException((boost::format("Failed to bind object parameter full path %1%. SQLite error %2%") % fullPath % bindResult).str());
		}
	}

	const auto stepResult = sqlite3_step(_insertObjectStatement);
	if (stepResult != SQLITE_DONE)
	{
		if (stepResult == SQLITE_CONSTRAINT)
		{
			throw DuplicateObjectException(info.address);
		}
		throw AddObjectFailedException((boost::format("Failed to execute statement for insert object %1%. SQLite error %2%") % info.address.ToString() % stepResult).str());
	}
}

FileObjectInfo FileObjectInfoRepository::GetObject(const ObjectAddress& address) const
{
	const auto binaryAddress = address.ToBinary();
	sqlitepp::ScopedStatementReset reset(_getObjectStatement);

	const auto index = sqlite3_bind_parameter_index(_getObjectStatement, ":Address");
	const auto bindResult = sqlite3_bind_blob(_getObjectStatement, index, &binaryAddress[0], static_cast<int>(binaryAddress.size()), 0);
	if (bindResult != SQLITE_OK)
	{
		throw ObjectNotFoundException((boost::format("Failed to bind object parameter address %1%. SQLite error %2%") % address.ToString() % bindResult).str());
	}

	std::string type;
	bool found = false;

	auto stepResult = sqlite3_step(_getObjectStatement);
	if (stepResult != SQLITE_ROW)
	{
		throw ObjectNotFoundException((boost::format("Object with address %1% not found.") % address.ToString()).str());
	}

	return *MapRowToObject(_getObjectStatement);
}

std::shared_ptr<FileObjectInfo> FileObjectInfoRepository::MapRowToObject(const sqlitepp::ScopedStatement& statement) const
{
	const auto objectAddressBytes = sqlite3_column_blob(statement, GetObject_ColumnIndex_Address);
	const auto objectAddressBytesCount = sqlite3_column_bytes(statement, GetObject_ColumnIndex_Address);
	const ObjectAddress objectAddress(objectAddressBytes, objectAddressBytesCount);

	const auto rawFullPath = sqlite3_column_text(statement, GetObject_ColumnIndex_FullPath);
	const auto fullPath = std::string(reinterpret_cast<const char*>(rawFullPath));
	
	const auto contentBlobAddressBytes = sqlite3_column_blob(statement, GetObject_ColumnIndex_ContentBlobAddress);
	const auto contentBlobAddressBytesCount = sqlite3_column_bytes(statement, GetObject_ColumnIndex_ContentBlobAddress);
	const BlobAddress contentBlobAddress(contentBlobAddressBytes, contentBlobAddressBytesCount);

	return std::make_shared<FileObjectInfo>(objectAddress, fullPath, contentBlobAddress);
}

}
}
}