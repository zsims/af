#include "bslib/BackupDatabase.hpp"

#include "bslib/exceptions.hpp"
#include "bslib/blob/BlobStore.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "bslib/BackupDatabaseUnitOfWork.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace af {
namespace bslib {

BackupDatabase::BackupDatabase(const boost::filesystem::path& forestPath, std::unique_ptr<blob::BlobStore> blobStore)
	: _forestPath(forestPath)
	, _blobStore(std::move(blobStore))
{
}

BackupDatabase::~BackupDatabase()
{
	// Needed to delete incomplete types
}

void BackupDatabase::Open()
{
	if (!boost::filesystem::exists(_forestPath))
	{
		throw DatabaseNotFoundException(_forestPath.string());
	}

	// Share the connection between the repos, note that the repos should be destroyed before this connection is
	_connection.reset(new sqlitepp::ScopedSqlite3Object());
	sqlitepp::open_database_or_throw(_forestPath.string().c_str(), *_connection, SQLITE_OPEN_READWRITE);
	sqlitepp::exec_or_throw(*_connection, "PRAGMA case_sensitive_like = true;");
}

void BackupDatabase::Create()
{
	{
		if (boost::filesystem::exists(_forestPath))
		{
			throw DatabaseAlreadyExistsException(_forestPath.string());
		}

		sqlitepp::ScopedSqlite3Object db;
		const auto result = sqlite3_open_v2(_forestPath.string().c_str(), db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
		if (result != SQLITE_OK)
		{
			throw CreateDatabaseFailedException(_forestPath.string(), result);
		}

		// Create tables
		// Note that SQLite supports blobs as primary keys fine, see https://www.sqlite.org/cvstrac/wiki?p=KeyValueDatabase
		const auto sql = R"(
			CREATE TABLE Blob (
				Address BLOB (20) PRIMARY KEY,
				SizeBytes INTEGER (8) NOT NULL
			);
			CREATE TABLE FileEvent (
				Id INTEGER PRIMARY KEY AUTOINCREMENT,
				FullPath TEXT NOT NULL COLLATE BINARY,
				ContentBlobAddress BLOB(20) REFERENCES Blob (Address),
				Action INTEGER NOT NULL,
				FileType INTEGER NOT NULL
			);
		)";

		sqlitepp::ScopedErrorMessage errorMessage;
		const auto execResult = sqlite3_exec(db, sql, 0, 0, errorMessage);
		if (execResult != SQLITE_OK)
		{
			throw CreateDatabaseFailedException(_forestPath.string(), execResult, errorMessage);
		}
	}

	Open();
}

void BackupDatabase::OpenOrCreate()
{
	if (boost::filesystem::exists(_forestPath))
	{
		Open();
		return;
	}
	Create();
}

std::unique_ptr<UnitOfWork> BackupDatabase::CreateUnitOfWork()
{
	return std::make_unique<BackupDatabaseUnitOfWork>(*_connection, *_blobStore);
}

std::unique_ptr<UnitOfWork> BackupDatabase::CreateUnitOfWork(blob::BlobStore& blobStore)
{
	return std::make_unique<BackupDatabaseUnitOfWork>(*_connection, blobStore);
}

}
}

