#pragma once

#include "ffs/address.hpp"
#include "ffs/blob/BlobInfo.hpp"

#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <vector>

namespace af {
namespace ffs {
namespace blob {

/**
 * Maintains blob information
 */
class BlobInfoRepository
{
public:
	/**
	 * Returns all of the blobs known.
	 */
	std::vector<BlobInfoModelPtr> GetAllBlobs() const;

	/**
	 * Adds a blob.
	 * \throws DuplicateBlobException if the address has already been stored
	 */
	void AddBlob(const BlobInfo& info);

private:
	std::map<BlobAddress, BlobInfoModelPtr> _things;
};

}
}
}