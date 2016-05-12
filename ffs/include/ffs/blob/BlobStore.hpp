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

	/**
	 * Creates a new blob.
	 */
	virtual BlobAddress CreateBlob(const std::vector<uint8_t>& content) = 0;

	/**
	 * Gets a blob by address.
	 * \exception BlobReadException The blob with the given address couldn't be read, e.g. it doesn't exist or a permissions failure.
	 */
	virtual std::vector<uint8_t> GetBlob(const BlobAddress& address) = 0;

protected:
	std::shared_ptr<BlobInfoRepository> _repository;
};

}
}
}

