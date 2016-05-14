#include "ffs/Forest.hpp"

#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/object/ObjectInfoRepository.hpp"

#include <ctime>

namespace af {
namespace ffs {

Forest::Forest()
	: _blobInfoRepository(new blob::BlobInfoRepository())
	, _objectInfoRepository(new object::ObjectInfoRepository())
	, _random(static_cast<unsigned>(time(0)))
{
}

ObjectAddress Forest::CreateObject(const std::string& type, const object::ObjectBlobList& objectBlobs)
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
	_objectInfoRepository->AddObject(info);
	return address;
}

object::ObjectInfo Forest::GetObject(const ObjectAddress& address) const
{
	return _objectInfoRepository->GetObject(address);
}

}
}

