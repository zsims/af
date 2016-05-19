#include "ffs/object/ObjectInfoRepository.hpp"

#include "ffs/object/ObjectInfo.hpp"
#include "ffs/object/exceptions.hpp"
#include "ffs/sqlitepp/sqlitepp.hpp"

#include <boost/format.hpp>
#include <sqlite3.h>

#include <memory>
#include <utility>

namespace af {
namespace ffs {
namespace object {

namespace {
enum GetObjectColumnIndex
{
	GetObject_ColumnIndex_ObjectAddress = 0,
	GetObject_ColumnIndex_ObjectType,
	GetObject_ColumnIndex_ObjectBlobBlobAddress,
	GetObject_ColumnIndex_ObjectBlobKey,
	GetObject_ColumnIndex_ObjectBlobPosition
};

enum GetAllObjectsColumnIndex
{
	GetAllObjects_ColumnIndex_ObjectAddress = 0,
	GetAllObjects_ColumnIndex_ObjectType,
	GetAllObjects_ColumnIndex_ObjectBlobBlobAddress,
	GetAllObjects_ColumnIndex_ObjectBlobKey,
	GetAllObjects_ColumnIndex_ObjectBlobPosition
};
}

ObjectInfoRepository::ObjectInfoRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	// Prepare statements so they're good to go
	sqlitepp::prepare_or_throw(_db, "INSERT INTO Object (Address, Type) VALUES (:Address, :Type)", _insertObjectStatement);
	sqlitepp::prepare_or_throw(_db, "INSERT INTO ObjectBlob (ObjectAddress, BlobAddress, Key, Position) VALUES (:ObjectAddress, :BlobAddress, :Key, :Position)", _insertObjectBlobStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Object.Address, Object.Type, ObjectBlob.BlobAddress, ObjectBlob.Key, ObjectBlob.Position FROM Object
		LEFT OUTER JOIN ObjectBlob ON Object.Address = ObjectBlob.ObjectAddress
		WHERE Object.Address = :Address
		ORDER BY ObjectBlob.Position ASC
	)", _getObjectStatement);
	sqlitepp::prepare_or_throw(_db, R"(
		SELECT Object.Address, Object.Type, ObjectBlob.BlobAddress, ObjectBlob.Key, ObjectBlob.Position FROM Object
		LEFT OUTER JOIN ObjectBlob ON Object.Address = ObjectBlob.ObjectAddress
		ORDER BY Object.Address, ObjectBlob.Position ASC
	)", _getAllObjectsStatement);
}

std::vector<ObjectInfoPtr> ObjectInfoRepository::GetAllObjects() const
{
	std::vector<ObjectInfoPtr> result;
	sqlitepp::ScopedTransaction transaction(_db);
	sqlitepp::ScopedStatementReset reset(_getAllObjectsStatement);

	ObjectAddress currentAddress;
	std::string currentType;
	ObjectBlobList currentObjectBlobs;
	bool haveOpenObject = false;

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getAllObjectsStatement)) == SQLITE_ROW)
	{
		const auto objectAddressBytes = sqlite3_column_blob(_getAllObjectsStatement, GetAllObjects_ColumnIndex_ObjectAddress);
		const auto objectAddressBytesCount = sqlite3_column_bytes(_getAllObjectsStatement, GetAllObjects_ColumnIndex_ObjectAddress);
		ObjectAddress objectAddress(objectAddressBytes, objectAddressBytesCount);

		if (!haveOpenObject)
		{
			currentAddress = objectAddress;
		}

		if (haveOpenObject && objectAddress != currentAddress)
		{
			// close off the previous object
			result.push_back(std::make_unique<ObjectInfo>(ObjectAddress(currentAddress), currentType, currentObjectBlobs));
			currentAddress = objectAddress;
			currentObjectBlobs = ObjectBlobList();
			currentType = "";
		}

		const auto rawType = sqlite3_column_text(_getAllObjectsStatement, GetAllObjects_ColumnIndex_ObjectType);
		currentType = std::string(reinterpret_cast<const char*>(rawType));
		haveOpenObject = true;
		
		// We have an object blob, and it's in order of position (low -> high)
		const auto blobAddressBytes = sqlite3_column_blob(_getAllObjectsStatement, GetAllObjects_ColumnIndex_ObjectBlobBlobAddress);
		if (blobAddressBytes == 0)
		{
			// no blob details, just skip
			continue;
		}

		const auto blobAddressBytesCount = sqlite3_column_bytes(_getAllObjectsStatement, GetAllObjects_ColumnIndex_ObjectBlobBlobAddress);
		const auto rawKey = reinterpret_cast<const char*>(sqlite3_column_text(_getAllObjectsStatement, GetAllObjects_ColumnIndex_ObjectBlobKey));
		currentObjectBlobs.push_back(std::make_pair(std::string(rawKey), BlobAddress(blobAddressBytes, blobAddressBytesCount)));
	}

	if (haveOpenObject)
	{
		result.push_back(std::make_unique<ObjectInfo>(ObjectAddress(currentAddress), currentType, currentObjectBlobs));
	}

	transaction.Rollback();
	return result;
}

void ObjectInfoRepository::InsertObjectBlobs(const binary_address& objectAddress, const ObjectBlobList& objectBlobs)
{
	const auto objectAddressIndex = sqlite3_bind_parameter_index(_insertObjectBlobStatement, ":ObjectAddress");
	const auto blobAddressIndex = sqlite3_bind_parameter_index(_insertObjectBlobStatement, ":BlobAddress");
	const auto keyIndex = sqlite3_bind_parameter_index(_insertObjectBlobStatement, ":Key");
	const auto positionIndex = sqlite3_bind_parameter_index(_insertObjectBlobStatement, ":Position");

	auto position = 0;
	for (const auto& ob : objectBlobs)
	{
		sqlitepp::ScopedStatementReset reset(_insertObjectBlobStatement);
		const auto& key = ob.first;
		const auto& blobAddress = ob.second;
		const auto binaryBlobAddress = blobAddress.ToBinary();

		{
			const auto bindResult = sqlite3_bind_blob(_insertObjectBlobStatement, objectAddressIndex, &objectAddress[0], static_cast<int>(objectAddress.size()), 0);
			if (bindResult != SQLITE_OK)
			{
				const auto ffs = sqlite3_errmsg(_db);
				throw AddObjectFailedException((boost::format("Failed to bind object blob parameter object address. SQLite error %1%: %2%") % bindResult % ffs).str());
			}
		}
		{
			const auto bindResult = sqlite3_bind_blob(_insertObjectBlobStatement, blobAddressIndex, &binaryBlobAddress[0], static_cast<int>(binaryBlobAddress.size()), 0);
			if (bindResult != SQLITE_OK)
			{
				throw AddObjectFailedException((boost::format("Failed to bind object blob parameter binary blob address. SQLite error %1%") % bindResult).str());
			}
		}
		{
			const auto bindResult = sqlite3_bind_text(_insertObjectBlobStatement, keyIndex, key.c_str(), -1, 0);
			if (bindResult != SQLITE_OK)
			{
				throw AddObjectFailedException((boost::format("Failed to bind object blob parameter key. SQLite error %1%") % bindResult).str());
			}
		}
		{
			const auto bindResult = sqlite3_bind_int(_insertObjectBlobStatement, positionIndex, position);
			if (bindResult != SQLITE_OK)
			{
				throw AddObjectFailedException((boost::format("Failed to bind object blob parameter position. SQLite error %1%") % bindResult).str());
			}
		}

		const auto stepResult = sqlite3_step(_insertObjectBlobStatement);
		if (stepResult != SQLITE_DONE)
		{
			throw AddObjectFailedException((boost::format("Failed to execute statement for insert object blob. SQLite error %1%") % stepResult).str());
		}

		position++;
	}
}

void ObjectInfoRepository::AddObject(const ObjectInfo& info)
{
	// binary address, note this has to be kept in scope until SQLite has finished as we've opted not to make a copy
	const auto binaryAddress = info.GetAddress().ToBinary();
	const auto type = info.GetType();
	sqlitepp::ScopedTransaction transaction(_db);
	sqlitepp::ScopedStatementReset reset(_insertObjectStatement);
	
	{
		const auto index = sqlite3_bind_parameter_index(_insertObjectStatement, ":Address");
		const auto bindResult = sqlite3_bind_blob(_insertObjectStatement, index, &binaryAddress[0], static_cast<int>(binaryAddress.size()), 0);
		if (bindResult != SQLITE_OK)
		{
			throw AddObjectFailedException((boost::format("Failed to bind object parameter address %1%. SQLite error %2%") % info.GetAddress().ToString() % bindResult).str());
		}
	}

	{
		const auto index = sqlite3_bind_parameter_index(_insertObjectStatement, ":Type");
		const auto bindResult = sqlite3_bind_text(_insertObjectStatement, index, type.c_str(), -1, 0);
		if (bindResult != SQLITE_OK)
		{
			throw AddObjectFailedException((boost::format("Failed to bind object parameter type %1%. SQLite error %2%") % type % bindResult).str());
		}
	}

	const auto stepResult = sqlite3_step(_insertObjectStatement);
	if (stepResult != SQLITE_DONE)
	{
		if (stepResult == SQLITE_CONSTRAINT)
		{
			throw DuplicateObjectException(info.GetAddress());
		}
		throw AddObjectFailedException((boost::format("Failed to execute statement for insert blob for object %1%. SQLite error %2%") % info.GetAddress().ToString() % stepResult).str());
	}

	InsertObjectBlobs(binaryAddress, info.GetBlobs());

	transaction.Commit();
}

ObjectInfo ObjectInfoRepository::GetObject(const ObjectAddress& address) const
{
	const auto binaryAddress = address.ToBinary();
	sqlitepp::ScopedTransaction transaction(_db);
	sqlitepp::ScopedStatementReset reset(_getObjectStatement);

	const auto index = sqlite3_bind_parameter_index(_getObjectStatement, ":Address");
	const auto bindResult = sqlite3_bind_blob(_getObjectStatement, index, &binaryAddress[0], static_cast<int>(binaryAddress.size()), 0);
	if (bindResult != SQLITE_OK)
	{
		throw ObjectNotFoundException((boost::format("Failed to bind object parameter address %1%. SQLite error %2%") % address.ToString() % bindResult).str());
	}

	std::string type;
	bool found = false;
	ObjectBlobList objectBlobs;

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getObjectStatement)) == SQLITE_ROW)
	{
		const auto rawType = sqlite3_column_text(_getObjectStatement, GetObject_ColumnIndex_ObjectType);
		if (!found)
		{
			type = std::string(reinterpret_cast<const char*>(rawType));
			found = true;
		}

		// We have a object blob, and it's in order of position (low -> high)
		const auto blobAddressBytes = sqlite3_column_blob(_getObjectStatement, GetObject_ColumnIndex_ObjectBlobBlobAddress);
		if (blobAddressBytes == 0)
		{
			// no blob details, just skip
			continue;
		}

		const auto blobAddressBytesCount = sqlite3_column_bytes(_getObjectStatement, GetObject_ColumnIndex_ObjectBlobBlobAddress);
		const auto rawKey = reinterpret_cast<const char*>(sqlite3_column_text(_getObjectStatement, GetObject_ColumnIndex_ObjectBlobKey));
		objectBlobs.push_back(std::make_pair(std::string(rawKey), BlobAddress(blobAddressBytes, blobAddressBytesCount)));
	}

	if (!found)
	{
		throw ObjectNotFoundException((boost::format("Object with address %1% not found.") % address.ToString()).str());
	}

	transaction.Rollback();
	return ObjectInfo(address, type, objectBlobs);
}

}
}
}