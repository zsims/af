#include "bslib/BackupDatabaseUnitOfWork.hpp"

#include <ctime>

namespace af {
namespace bslib {

BackupDatabaseUnitOfWork::BackupDatabaseUnitOfWork(PooledDatabaseConnection connection, std::shared_ptr<blob::BlobStore> blobStore)
	: _connection(std::move(connection))
	, _transaction(_connection->GetSqlConnection())
	, _blobStore(blobStore)
{
}

void BackupDatabaseUnitOfWork::Commit()
{
	_transaction.Commit();
}

std::unique_ptr<file::FileBackupRunRecorder> BackupDatabaseUnitOfWork::CreateFileBackupRunRecorder()
{
	return std::make_unique<file::FileBackupRunRecorder>(_connection->GetFileBackupRunEventStreamRepository());
}

std::unique_ptr<file::FileAdder> BackupDatabaseUnitOfWork::CreateFileAdder()
{
	return std::make_unique<file::FileAdder>(_blobStore, _connection->GetBlobInfoRepository(), _connection->GetFileEventStreamRepository());
}

std::unique_ptr<file::FileRestorer> BackupDatabaseUnitOfWork::CreateFileRestorer()
{
	return std::make_unique<file::FileRestorer>(_blobStore, _connection->GetBlobInfoRepository());
}

std::unique_ptr<file::FileFinder> BackupDatabaseUnitOfWork::CreateFileFinder()
{
	return std::make_unique<file::FileFinder>(_connection->GetFileEventStreamRepository());
}

std::vector<uint8_t> BackupDatabaseUnitOfWork::GetBlob(const blob::Address& address) const
{
	// TODO: does this need to be tracked/looked up?
	return _blobStore->GetBlob(address);
}

}
}