#pragma once

#include "ffs/Address.hpp"
#include "ffs/object/ObjectInfo.hpp"

#include <boost/core/noncopyable.hpp>

namespace af {
namespace ffs {

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
	virtual ObjectAddress CreateObject(const std::string& type, const object::ObjectBlobList& objectBlobs) = 0;

	/**
	 * Gets an object by address.
	 * \throws ObjectNotFoundException No object with the given address could be found.
	 */
	virtual object::ObjectInfo GetObject(const ObjectAddress& address) const = 0;

	/**
	 * Creates a new blob.
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
