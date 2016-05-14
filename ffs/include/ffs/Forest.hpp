#pragma once

#include "ffs/Address.hpp"
#include "ffs/object/ObjectInfo.hpp"

#include <memory>
#include <string>
#include <random>

namespace af {
namespace ffs {

namespace blob {
class BlobInfoRepository;
}

namespace object {
class ObjectInfoRepository;
}

class Forest
{
public:
	Forest();

	/**
	 * Creates a new object.
	 */
	ObjectAddress CreateObject(const std::string& type, const object::ObjectBlobList& objectBlobs);

	/**
	 * Gets an object by address.
	 * \throws ObjectNotFoundException No object with the given address could be found.
	 */
	object::ObjectInfo GetObject(const ObjectAddress& address) const;

	std::shared_ptr<blob::BlobInfoRepository> GetBlobInfoRepository() const { return _blobInfoRepository; }
private:
	std::shared_ptr<blob::BlobInfoRepository> _blobInfoRepository;
	std::shared_ptr<object::ObjectInfoRepository> _objectInfoRepository;
	std::mt19937 _random;
};

}
}