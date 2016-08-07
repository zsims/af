#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileObjectInfo.hpp"

#include <boost/core/noncopyable.hpp>

#include <string>

namespace af {
namespace bslib {

class UnitOfWork : private boost::noncopyable
{
public:
	UnitOfWork() { }
	virtual ~UnitOfWork() { }

	/**
	 * Saves the new unit of work
	 */
	virtual void Commit() = 0;

	/**
	 * Creates a new object.
	 */
	virtual ObjectAddress CreateFileObject(const std::string& fullPath, const BlobAddress& contentBlobAddress) = 0;

	/**
	 * Gets a file object by address.
	 * \throws ObjectNotFoundException No object with the given address could be found.
	 */
	virtual file::FileObjectInfo GetFileObject(const ObjectAddress& address) const = 0;

	/**
	 * Creates a new blob. If a blob with the same content already exists, then its address is returned.
	 */
	virtual BlobAddress CreateBlob(const std::vector<uint8_t>& content) = 0;

	/**
	 * Gets a blob by address.
	 * \exception BlobReadException The blob with the given address couldn't be read, e.g. it doesn't exist or a permissions failure.
	 */
	virtual std::vector<uint8_t> GetBlob(const BlobAddress& address) const = 0;
};


}
}
