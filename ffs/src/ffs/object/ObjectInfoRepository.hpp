#pragma once

#include "ffs/Address.hpp"
#include "ffs/object/ObjectInfo.hpp"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace af {
namespace ffs {
namespace object {

/**
 * Maintains object information
 */
class ObjectInfoRepository
{
public:
	std::vector<ObjectInfoPtr> GetAllObjects() const;

	void AddObject(const ObjectInfo& info);
private:
	std::map<ObjectAddress, ObjectInfoPtr> _objects;
};

typedef std::shared_ptr<ObjectInfoRepository> ObjectInfoRepositoryPtr;

}
}
}