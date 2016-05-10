#pragma once

#include "ffs/Address.hpp"

#include <memory>
#include <vector>

namespace af {
namespace ffs {
namespace blob {

class BlobInfoRepository;

class BlobStore
{
public:
	explicit BlobStore(std::shared_ptr<BlobInfoRepository> repository);

	virtual BlobAddress CreateBlob(const std::vector<uint8_t>& content) = 0;

protected:
	BlobAddress CalculateAddress(const std::vector<uint8_t>& content);

	std::shared_ptr<BlobInfoRepository> _repository;
};

}
}
}

