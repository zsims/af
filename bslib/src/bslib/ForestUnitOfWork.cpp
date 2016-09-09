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
	, _fileEventStreamRepository(connection)
{
}

void ForestUnitOfWork::Commit()
{
	_transaction.Commit();
}

std::unique_ptr<file::FileAdderEs> ForestUnitOfWork::CreateFileAdderEs()
{
	return std::make_unique<file::FileAdderEs>(_blobStore, _blobInfoRepository, _fileEventStreamRepository);
}

std::unique_ptr<file::FileRestorerEs> ForestUnitOfWork::CreateFileRestorerEs()
{
	return std::make_unique<file::FileRestorerEs>(_blobStore, _blobInfoRepository);
}

std::unique_ptr<file::FileFinder> ForestUnitOfWork::CreateFileFinder()
{
	return std::make_unique<file::FileFinder>(_fileObjectRepository, _fileRefRepository, _fileEventStreamRepository);
}

std::vector<uint8_t> ForestUnitOfWork::GetBlob(const BlobAddress& address) const
{
	// TODO: does this need to be tracked/looked up?
	return _blobStore.GetBlob(address);
}

}
}