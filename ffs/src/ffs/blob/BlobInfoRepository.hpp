#pragma once

#include "ffs/address.hpp"
#include "ffs/blob/BlobInfo.hpp"
#include "ffs/sqlitepp/handles.hpp"

#include <cstdint>
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
	 * Creates a new blob info repository given an existing database connection
	 */
	explicit BlobInfoRepository(const sqlitepp::ScopedSqlite3Object& connection);

	/**
	 * Returns all of the blobs known.
	 */
	std::vector<std::shared_ptr<BlobInfo>> GetAllBlobs() const;

	/**
	 * Adds a blob.
	 * \throws DuplicateBlobException if the address has already been stored
	 */
	void AddBlob(const BlobInfo& info);
private:
	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _getAllBlobsStatement;
	sqlitepp::ScopedStatement _insertBlobStatement;
};

}
}
}