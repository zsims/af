#pragma once

#include "bslib/address.hpp"
#include "bslib/blob/BlobInfo.hpp"
#include "bslib/sqlitepp/handles.hpp"

#include <cstdint>
#include <memory>
#include <vector>

namespace af {
namespace bslib {
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

	/**
	 * Finds a blob by address.
	 * \return a NULL pointer if the blob couldn't be found, else its information.
	 */
	std::unique_ptr<BlobInfo> FindBlob(const BlobAddress& address);
private:
	const sqlitepp::ScopedSqlite3Object& _db;
	sqlitepp::ScopedStatement _getAllBlobsStatement;
	sqlitepp::ScopedStatement _insertBlobStatement;
	sqlitepp::ScopedStatement _findBlobStatement;
};

}
}
}