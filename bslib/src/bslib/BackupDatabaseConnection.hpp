#pragma once

#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/FileBackupRunEventStreamRepository.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/ObjectPool.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <boost/core/noncopyable.hpp>

namespace af {
namespace bslib {

/**
 * Represents a connection to the backup database
 * Additionally creates the repositories to take advantage of prepared statements that are connection-specific
 */
class BackupDatabaseConnection : public boost::noncopyable
{
public:
	explicit BackupDatabaseConnection(std::unique_ptr<sqlitepp::ScopedSqlite3Object> connection)
		: _connection(std::move(connection))
		, _blobInfoRepository(*_connection)
		, _fileEventStreamRepository(*_connection)
		, _backupRunEventStreamRepository(*_connection)
	{
	}

	const sqlitepp::ScopedSqlite3Object& GetSqlConnection() { return *_connection; }
	blob::BlobInfoRepository& GetBlobInfoRepository() { return _blobInfoRepository; }
	file::FileEventStreamRepository& GetFileEventStreamRepository() { return _fileEventStreamRepository; }
	file::FileBackupRunEventStreamRepository& GetFileBackupRunEventStreamRepository() { return _backupRunEventStreamRepository; }

private:
	const std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	blob::BlobInfoRepository _blobInfoRepository;
	file::FileEventStreamRepository _fileEventStreamRepository;
	file::FileBackupRunEventStreamRepository _backupRunEventStreamRepository;
};

typedef ObjectPool<BackupDatabaseConnection>::PointerType PooledDatabaseConnection;

}
}
