#include "bslib/ForestUnitOfWork.hpp"

#include <ctime>

namespace af {
namespace bslib {

ForestUnitOfWork::ForestUnitOfWork(const sqlitepp::ScopedSqlite3Object& connection, blob::BlobStore& blobStore)
	: _transaction(connection)
	, _blobStore(blobStore)
	, _blobInfoRepository(connection)
	, _fileObjectInfoRepository(connection)
{
}

void ForestUnitOfWork::Commit()
{
	_transaction.Commit();
}

std::unique_ptr<file::FileAdder> ForestUnitOfWork::CreateFileAdder()
{
	return std::make_unique<file::FileAdder>(_blobStore, _blobInfoRepository, _fileObjectInfoRepository);
}

file::FileObjectInfo ForestUnitOfWork::GetFileObject(const ObjectAddress& address) const
{
	return _fileObjectInfoRepository.GetObject(address);
}

std::vector<uint8_t> ForestUnitOfWork::GetBlob(const BlobAddress& address) const
{
	// TODO: does this need to be tracked/looked up?
	return _blobStore.GetBlob(address);
}

}
}