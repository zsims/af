#include "ffs/ForestUnitOfWork.hpp"

#include <ctime>

namespace af {
namespace ffs {

ForestUnitOfWork::ForestUnitOfWork(
	sqlite3* connection,
	blob::BlobStore& blobStore,
	blob::BlobInfoRepository& blobInfoRepository,
	object::ObjectInfoRepository& objectInfoRepository)
	: _random(static_cast<unsigned>(time(0)))
	, _transaction(connection)
	, _blobStore(blobStore)
	, _blobInfoRepository(blobInfoRepository)
	, _objectInfoRepository(objectInfoRepository)
{
}

void ForestUnitOfWork::Commit()
{
	_transaction.Commit();
}

ObjectAddress ForestUnitOfWork::CreateObject(const std::string& type, const object::ObjectBlobList& objectBlobs)
{
	auto r = [this]() {
		return static_cast<uint8_t>(_random());
	};
	// generate a new random address
	ObjectAddress address(binary_address{
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r(),
		r(), r(), r(), r(), r()
	});
	object::ObjectInfo info(address, type, objectBlobs);
	_objectInfoRepository.AddObject(info);

	return address;
}

object::ObjectInfo ForestUnitOfWork::GetObject(const ObjectAddress& address) const
{
	return _objectInfoRepository.GetObject(address);
}

BlobAddress ForestUnitOfWork::CreateBlob(const std::vector<uint8_t>& content)
{
	const auto address = BlobAddress::CalculateFromContent(content);
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