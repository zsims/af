#pragma once

#include "bslib/blob/Address.hpp"

#include <memory>
#include <vector>

namespace af {
namespace bslib {
namespace blob {

class BlobStore
{
public:
	/**
	 * Creates a new blob.
	 */
	virtual void CreateBlob(const Address& address, const std::vector<uint8_t>& content) = 0;

	/**
	 * Gets a blob by address.
	 * \exception BlobReadException The blob with the given address couldn't be read, e.g. it doesn't exist or a permissions failure.
	 */
	virtual std::vector<uint8_t> GetBlob(const Address& address) const = 0;
};

}
}
}

