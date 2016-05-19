#pragma once

#include "ffs/address.hpp"
#include "ffs/blob/BlobInfo.hpp"
#include "ffs/sqlitepp/handles.hpp"

#include <cstdint>
#include <memory>
#include <string>
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
	 * Creates a new blob info repository given an existing database path.
	 */
	explicit BlobInfoRepository(const std::string& utf8DbPath);

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
	sqlitepp::ScopedSqlite3Object _db;
	sqlitepp::ScopedStatement _getAllBlobsStatement;
	sqlitepp::ScopedStatement _insertBlobStatement;
};

typedef std::shared_ptr<BlobInfoRepository> BlobInfoRepositoryPtr;

}
}
}