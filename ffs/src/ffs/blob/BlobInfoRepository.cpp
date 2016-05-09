#include "ffs/blob/BlobInfoRepository.hpp"

#include "ffs/blob/BlobInfo.hpp"
#include "ffs/blob/exceptions.hpp"

#include <memory>

namespace af {
namespace ffs {
namespace blob {

std::vector<BlobInfoModelPtr> BlobInfoRepository::GetAllBlobs() const
{
	std::vector<BlobInfoModelPtr> result;
	result.reserve(_things.size());

	for (const auto& kv : _things)
	{
		result.push_back(kv.second);
	}

	return result;
}

void BlobInfoRepository::AddBlob(const BlobInfo& info)
{
	auto it = _things.find(info.GetAddress());
	if (it != _things.end())
	{
		throw DuplicateBlobException("Something");
	}

	_things.insert(std::make_pair(info.GetAddress(), std::make_shared<BlobInfo>(info)));
}

}
}
}