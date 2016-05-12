#include "ffs/object/ObjectInfoRepository.hpp"

#include "ffs/object/ObjectInfo.hpp"
#include "ffs/object/exceptions.hpp"

#include <memory>

namespace af {
namespace ffs {
namespace object {

std::vector<ObjectInfoPtr> ObjectInfoRepository::GetAllObjects() const
{
	std::vector<ObjectInfoPtr> result;
	result.reserve(_objects.size());

	for (auto kv : _objects)
	{
		result.push_back(kv.second);
	}

	return result;
}

void ObjectInfoRepository::AddObject(const ObjectInfo& info)
{
	auto it = _objects.find(info.GetAddress());
	if (it != _objects.end())
	{
		throw DuplicateObjectException("Something");
	}

	// TODO: other things...
	_objects.insert(std::make_pair(info.GetAddress(), std::make_shared<ObjectInfo>(info)));
}

ObjectInfo ObjectInfoRepository::GetObject(const ObjectAddress& address) const
{
	auto it = _objects.find(address);
	if (it == _objects.end())
	{
		throw ObjectNotFoundException(address);
	}

	return *(it->second);

}

}
}
}