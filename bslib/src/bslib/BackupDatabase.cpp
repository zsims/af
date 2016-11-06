#include "bslib/BackupDatabase.hpp"

#include "bslib/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "bslib/BackupDatabaseUnitOfWork.hpp"

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

namespace af {
namespace bslib {

BackupDatabase::BackupDatabase(const boost::filesystem::path& databasePath)
	: _databasePath(databasePath)
	, _connections(3, [&]() { return Connect(); })
{
}

BackupDatabase::~BackupDatabase()
{
	// Needed to delete incomplete types
}

std::unique_ptr<BackupDatabaseConnection> BackupDatabase::Connect()
{
	auto connection = std::make_unique<sqlitepp::ScopedSqlite3Object>();
	sqlitepp::open_database_or_throw(_databasePath.string().c_str(), *connection, SQLITE_OPEN_READWRITE);
	sqlitepp::exec_or_throw(*connection, "PRAGMA case_sensitive_like = true;");
	return std::make_unique<BackupDatabaseConnection>(std::move(connection));
}

void BackupDatabase::Open()
{
	if (!boost::filesystem::exists(_databasePath))
	{
		throw DatabaseNotFoundException(_databasePath.string());
	}

	_connections.AddOne();
}

void BackupDatabase::Create()
{
	{
		if (boost::filesystem::exists(_databasePath))
		{
			throw DatabaseAlreadyExistsException(_databasePath.string());
		}

		sqlitepp::ScopedSqlite3Object db;
		const auto result = sqlite3_open_v2(_databasePath.string().c_str(), db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
		if (result != SQLITE_OK)
		{
			throw CreateDatabaseFailedException(_databasePath.string(), result);
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
			throw CreateDatabaseFailedException(_databasePath.string(), execResult, errorMessage);
		}
	}

	Open();
}

void BackupDatabase::OpenOrCreate()
{
	if (boost::filesystem::exists(_databasePath))
	{
		Open();
		return;
	}
	Create();
}

std::unique_ptr<UnitOfWork> BackupDatabase::CreateUnitOfWork(blob::BlobStore& blobStore)
{
	auto pooledConnection = _connections.Acquire();
	return std::make_unique<BackupDatabaseUnitOfWork>(std::move(pooledConnection), blobStore);
}

}
}

