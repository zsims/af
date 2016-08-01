#include "ffs/Forest.hpp"

#include "ffs/exceptions.hpp"
#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/blob/BlobStore.hpp"
#include "ffs/object/FileObjectInfoRepository.hpp"
#include "ffs/sqlitepp/sqlitepp.hpp"
#include "ffs/ForestUnitOfWork.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace af {
namespace ffs {

Forest::Forest(const std::string& utf8DbPath, std::unique_ptr<blob::BlobStore> blobStore)
	: _utf8DbPath(utf8DbPath)
	, _blobStore(std::move(blobStore))
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
	sqlitepp::open_database_or_throw(_utf8DbPath.c_str(), *_connection, SQLITE_OPEN_READWRITE);
	_blobInfoRepository.reset(new blob::BlobInfoRepository(*_connection));
	_fileObjectInfoRepository.reset(new object::FileObjectInfoRepository(*_connection));
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
			throw CreateDatabaseFailedException(_utf8DbPath, result);
		}

		// Create tables
		// Note that SQLite supports blobs as primary keys fine, see https://www.sqlite.org/cvstrac/wiki?p=KeyValueDatabase
		const auto sql = R"(
			CREATE TABLE FileObject (
				Address BLOB (20) PRIMARY KEY,
				ContentBlobAddress BLOB(20) REFERENCES Blob (Address),
				FullPath TEXT NOT NULL
			);
			CREATE TABLE Blob (Address BLOB (20) PRIMARY KEY, SizeBytes INTEGER (8) NOT NULL);
		)";

		sqlitepp::ScopedErrorMessage errorMessage;
		const auto execResult = sqlite3_exec(db, sql, 0, 0, errorMessage);
		if (execResult != SQLITE_OK)
		{
			throw CreateDatabaseFailedException(_utf8DbPath, execResult, errorMessage);
		}
	}

	Open();
}


std::unique_ptr<UnitOfWork> Forest::CreateUnitOfWork()
{
	return std::make_unique<ForestUnitOfWork>(*_connection, *_blobStore, *_blobInfoRepository, *_fileObjectInfoRepository);
}


}
}

