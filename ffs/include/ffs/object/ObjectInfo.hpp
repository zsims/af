#pragma once

#include "ffs/Address.hpp"

#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace af {
namespace ffs {
namespace object {

typedef std::vector<std::pair<std::string, BlobAddress>> ObjectBlobList;

class ObjectInfo
{
public:
	explicit ObjectInfo(const ObjectAddress& address, const std::string& type, const ObjectBlobList& blobs)
		: _address(address)
		, _type(type)
		, _blobs(blobs)
	{
	}

	const ObjectAddress GetAddress() const { return _address; }
	const ObjectBlobList& GetBlobs() const { return _blobs; }
	const std::string GetType() const { return _type; }

	bool operator==(const ObjectInfo& rhs) const { return _address == rhs.GetAddress();}

private:
	const ObjectAddress _address;
	const std::string _type;
	const ObjectBlobList _blobs;
};

typedef std::shared_ptr<ObjectInfo> ObjectInfoPtr;

}
}
}