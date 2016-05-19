#include "ffs/Forest.hpp"

#include "ffs/exceptions.hpp"
#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/object/ObjectInfoRepository.hpp"
#include "ffs/sqlitepp/handles.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <ctime>

namespace af {
namespace ffs {

Forest::Forest(const std::string& utf8DbPath)
	: _utf8DbPath(utf8DbPath)
	, _random(static_cast<unsigned>(time(0)))
{
}

Forest::~Forest()
{
	// Needed to delete incomplete types
}

void Forest::Open()
{
	if (!boost::filesystem::exists(_utf8DbPath))
	{
		throw DatabaseNotFoundException(_utf8DbPath);
	}

	// Share the connection between the repos, note that the repos should be destroyed before this connection is
	_connection.reset(new sqlitepp::ScopedSqlite3Object());
	const auto result = sqlite3_open_v2(_utf8DbPath.c_str(), *_connection, SQLITE_OPEN_READWRITE, 0);
	if (result != SQLITE_OK)
	{
		throw OpenDatabaseFailedException((boost::format("Cannot open database at %1%. SQLite returned %2%") % _utf8DbPath % result).str());
	}
	_blobInfoRepository.reset(new blob::BlobInfoRepository(*_connection));
	_objectInfoRepository.reset(new object::ObjectInfoRepository(*_connection));
}

void Forest::Create()
{
	{
		if (boost::filesystem::exists(_utf8DbPath))
		{
			throw DatabaseAlreadyExistsException(_utf8DbPath);
		}

		sqlitepp::ScopedSqlite3Object db;
		const auto result = sqlite3_open_v2(_utf8DbPath.c_str(), db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
		if (result != SQLITE_OK)
		{
			throw CreateDatabaseFailedException((boost::format("Cannot create database at %1%. SQLite returned %2%") % _utf8DbPath % result).str());
		}

		// Create tables
		// Note that SQLite supports blobs as primary keys fine, see https://www.sqlite.org/cvstrac/wiki?p=KeyValueDatabase
		const auto sql = R"(
			CREATE TABLE Object (Address BLOB (20) PRIMARY KEY, Type TEXT NOT NULL);
			CREATE TABLE Blob (Address BLOB (20) PRIMARY KEY, SizeBytes INTEGER (8) NOT NULL);
			CREATE TABLE ObjectBlob (
				Id INTEGER PRIMARY KEY,
				ObjectAddress BLOB (20) REFERENCES Object (Address) ON DELETE CASCADE,
				Key TEXT NOT NULL,
				Position INTEGER NOT NULL,
				BlobAddress BLOB (20) REFERENCES Blob (Address)
			);
		)";

		sqlitepp::ScopedErrorMessage errorMessage;
		const auto execResult = sqlite3_exec(db, sql, 0, 0, errorMessage);
		if (execResult != SQLITE_OK)
		{
			throw CreateDatabaseFailedException((boost::format("Cannot create database at %1%. SQLite returned %2%: %3%") % _utf8DbPath % execResult % errorMessage).str());
		}
	}

	Open();
}

ObjectAddress Forest::CreateObject(const std::string& type, const object::ObjectBlobList& objectBlobs)
{
	auto r = [this]() {
		return static_cast<uint8_t>(_random());
	};
	// generate a new random address
	ObjectAddress address(binary_address{
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r()
	});
	object::ObjectInfo info(address, type, objectBlobs);
	_objectInfoRepository->AddObject(info);
	return address;
}

object::ObjectInfo Forest::GetObject(const ObjectAddress& address) const
{
	return _objectInfoRepository->GetObject(address);
}

}
}

