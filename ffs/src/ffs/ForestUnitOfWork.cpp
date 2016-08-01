#include "ffs/ForestUnitOfWork.hpp"

#include <ctime>

namespace af {
namespace ffs {

ForestUnitOfWork::ForestUnitOfWork(
	sqlite3* connection,
	blob::BlobStore& blobStore,
	blob::BlobInfoRepository& blobInfoRepository,
	object::FileObjectInfoRepository& fileObjectInfoRepository)
	: _random(static_cast<unsigned>(time(0)))
	, _transaction(connection)
	, _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
	, _fileObjectInfoRepository(fileObjectInfoRepository)
{
}

void ForestUnitOfWork::Commit()
{
	_transaction.Commit();
}

ObjectAddress ForestUnitOfWork::CreateFileObject(const std::string& fullPath, const BlobAddress& contentBlobAddress)
{
	auto r = [this]() {
		return static_cast<uint8_t>(_random());
	};
	// generate a new random address
	// TODO: should this come from a combination of the properties + blob address?
	ObjectAddress address(binary_address{
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r()
	});
	object::FileObjectInfo info(address, fullPath, contentBlobAddress);
	_fileObjectInfoRepository.AddObject(info);

	return address;
}

object::FileObjectInfo ForestUnitOfWork::GetFileObject(const ObjectAddress& address) const
{
	return _fileObjectInfoRepository.GetObject(address);
}

BlobAddress ForestUnitOfWork::CreateBlob(const std::vector<uint8_t>& content)
{
	const auto address = BlobAddress::CalculateFromContent(content);
	const auto existingBlob = _blobInfoRepository.FindBlob(address);
	if (existingBlob)
	{
		// TOOD: Should get the store to double check at least?
		return address;
	}

	_blobStore.CreateBlob(address, content);
	_blobInfoRepository.AddBlob(blob::BlobInfo(address, content.size()));
	return address;
}

std::vector<uint8_t> ForestUnitOfWork::GetBlob(const BlobAddress& address) const
{
	// TODO: does this need to be tracked/looked up?
	return _blobStore.GetBlob(address);
}

}
}