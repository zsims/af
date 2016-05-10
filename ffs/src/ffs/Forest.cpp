#include "ffs/Forest.hpp"

#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/object/ObjectInfoRepository.hpp"

#include <ctime>

namespace af {
namespace ffs {

Forest::Forest(const std::string& forestFile)
	: _forestFile(forestFile)
	, _blobInfoRepository(new blob::BlobInfoRepository())
	, _objectInfoRepository(new object::ObjectInfoRepository())
	, _random(static_cast<unsigned>(time(0)))
{
}

ObjectAddress Forest::CreateObject(const std::string& type, const object::ObjectBlobList& objectBlobs)
{
	// generate a new random address
	ObjectAddress address = { _random(), _random(), _random(), _random(), _random() };
	object::ObjectInfo info(address, type, objectBlobs);
	_objectInfoRepository->AddObject(info);
	return address;
}

}
}

