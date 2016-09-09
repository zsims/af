#pragma once

#include "bslib/blob/BlobStore.hpp"

namespace af {
namespace bslib {
namespace blob {

/**
 * Null implementation of the blob store where blobs are not stored.
 */
class NullBlobStore : public BlobStore
{
public:
	void CreateBlob(const Address& address, const std::vector<uint8_t>& content) override { }
	std::vector<uint8_t> GetBlob(const Address& address) const override { return std::vector<uint8_t>(); }
};

}
}
}
