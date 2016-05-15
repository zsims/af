#pragma once

#include "ffs/address.hpp"
#include "ffs/blob/BlobInfo.hpp"

#include <cstdint>
#include <exception>
#include <memory>
#include <string>
#include <vector>

struct sqlite3;

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
	 * Creates a new blob info repository given an existing database path.
	 */
	explicit BlobInfoRepository(const std::string& utf8DbPath);
	~BlobInfoRepository();

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
	sqlite3* _db;
};

typedef std::shared_ptr<BlobInfoRepository> BlobInfoRepositoryPtr;

}
}
}