#include "bslib/file/FileObjectInfoRepository.hpp"

#include "bslib/file/FileObjectInfo.hpp"
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

FileObjectInfoRepository::FileObjectInfoRepository(const sqlitepp::ScopedSqlite3Object& connection)
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
		if (info.contentBlobAddress)
		{
			const auto binaryContentAddress = info.contentBlobAddress.value().ToBinary();
			const auto bindResult = sqlite3_bind_blob(_insertObjectStatement, index, &binaryContentAddress[0], static_cast<int>(binaryContentAddress.size()), 0);
			if (bindResult != SQLITE_OK)
			{
				throw AddObjectFailedException((boost::format("Failed to bind object parameter content blob address %1%. SQLite error %2%") % info.contentBlobAddress.value().ToString() % bindResult).str());
			}
		}
		else
		{
			const auto bindResult = sqlite3_bind_null(_insertObjectStatement, index);
			if (bindResult != SQLITE_OK)
			{
				throw AddObjectFailedException((boost::format("Failed to bind object parameter content blob address null. SQLite error %1%") % bindResult).str());
			}
		}
	}

	{
		const auto index = sqlite3_bind_parameter_index(_insertObjectStatement, ":ParentAddress");
		if (info.parentAddress)
		{
			const auto binaryParentAddress = info.parentAddress.value().ToBinary();
			const auto bindResult = sqlite3_bind_blob(_insertObjectStatement, index, &binaryParentAddress[0], static_cast<int>(binaryParentAddress.size()), 0);
			if (bindResult != SQLITE_OK)
			{
				throw AddObjectFailedException((boost::format("Failed to bind object parameter parent address %1%. SQLite error %2%") % info.parentAddress.value().ToString() % bindResult).str());
			}
		}
		else
		{
			const auto bindResult = sqlite3_bind_null(_insertObjectStatement, index);
			if (bindResult != SQLITE_OK)
			{
				throw AddObjectFailedException((boost::format("Failed to bind object parameter parent address null. SQLite error %1%") % bindResult).str());
			}
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

	return std::make_shared<FileObjectInfo>(objectAddress, fullPath, contentBlobAddress, parentAddress);
}

}
}
}