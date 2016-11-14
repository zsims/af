#pragma once

#include "bslib/BackupDatabaseConnection.hpp"
#include "bslib/blob/Address.hpp"
#include "bslib/blob/BlobInfo.hpp"
#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/FileEvent.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/sqlitepp/ScopedTransaction.hpp"
#include "bslib/UnitOfWork.hpp"

#include <boost/core/noncopyable.hpp>

namespace af {
namespace bslib {

class BackupDatabaseUnitOfWork : public UnitOfWork
{
public:
	BackupDatabaseUnitOfWork(PooledDatabaseConnection connection, std::shared_ptr<blob::BlobStore> blobStore);

	void Commit() override;

	std::unique_ptr<file::FileBackupRunRecorder> CreateFileBackupRunRecorder() override;
	std::unique_ptr<file::FileAdder> CreateFileAdder() override;
	std::unique_ptr<file::FileRestorer> CreateFileRestorer() override;
	std::unique_ptr<file::FileFinder> CreateFileFinder() override;
	std::vector<uint8_t> GetBlob(const blob::Address& address) const override;
private:
	PooledDatabaseConnection _connection;
	sqlitepp::ScopedTransaction _transaction;
	std::shared_ptr<blob::BlobStore> _blobStore;
};


}
}