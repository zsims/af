#include "bslib/BackupDatabaseUnitOfWork.hpp"

#include <ctime>

namespace af {
namespace bslib {

BackupDatabaseUnitOfWork::BackupDatabaseUnitOfWork(const sqlitepp::ScopedSqlite3Object& connection, blob::BlobStore& blobStore)
	: _transaction(connection)
	, _blobStore(blobStore)
	, _blobInfoRepository(connection)
	, _fileEventStreamRepository(connection)
{
}

void BackupDatabaseUnitOfWork::Commit()
{
	_transaction.Commit();
}

std::unique_ptr<file::FileAdder> BackupDatabaseUnitOfWork::CreateFileAdder()
{
	return std::make_unique<file::FileAdder>(_blobStore, _blobInfoRepository, _fileEventStreamRepository);
}

std::unique_ptr<file::FileRestorer> BackupDatabaseUnitOfWork::CreateFileRestorer()
{
	return std::make_unique<file::FileRestorer>(_blobStore, _blobInfoRepository);
}

std::unique_ptr<file::FileFinder> BackupDatabaseUnitOfWork::CreateFileFinder()
{
	return std::make_unique<file::FileFinder>(_fileEventStreamRepository);
}

std::vector<uint8_t> BackupDatabaseUnitOfWork::GetBlob(const blob::Address& address) const
{
	// TODO: does this need to be tracked/looked up?
	return _blobStore.GetBlob(address);
}

}
}