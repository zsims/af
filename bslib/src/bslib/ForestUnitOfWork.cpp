#include "bslib/ForestUnitOfWork.hpp"

#include <ctime>

namespace af {
namespace bslib {

ForestUnitOfWork::ForestUnitOfWork(const sqlitepp::ScopedSqlite3Object& connection, blob::BlobStore& blobStore)
	: _transaction(connection)
	, _blobStore(blobStore)
	, _blobInfoRepository(connection)
	, _fileObjectRepository(connection)
	, _fileRefRepository(connection)
{
}

void ForestUnitOfWork::Commit()
{
	_transaction.Commit();
}

std::unique_ptr<file::FileAdder> ForestUnitOfWork::CreateFileAdder()
{
	return std::make_unique<file::FileAdder>(_blobStore, _blobInfoRepository, _fileObjectRepository, _fileRefRepository);
}

std::unique_ptr<file::FileRestorer> ForestUnitOfWork::CreateFileRestorer()
{
	return std::make_unique<file::FileRestorer>(_blobStore, _blobInfoRepository, _fileObjectRepository, _fileRefRepository);
}

std::unique_ptr<file::FileFinder> ForestUnitOfWork::CreateFileFinder()
{
	return std::make_unique<file::FileFinder>(_fileObjectRepository, _fileRefRepository);
}

std::vector<uint8_t> ForestUnitOfWork::GetBlob(const BlobAddress& address) const
{
	// TODO: does this need to be tracked/looked up?
	return _blobStore.GetBlob(address);
}

}
}