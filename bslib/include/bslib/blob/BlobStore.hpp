#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/unicode.hpp"
#include "bslib/Uuid.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>

#include <memory>
#include <vector>

namespace af {
namespace bslib {
namespace blob {

class BlobStore
{
public:
	virtual ~BlobStore() { }

	/**
	 * Gets a simple string that describes the type of this blob store
	 */
	virtual UTF8String GetTypeString() const = 0;

	/**
	 * Gets the id of this store
	 */
	virtual Uuid GetId() const = 0;

	/**
	 * Creates a new blob.
	 */
	virtual void CreateBlob(const Address& address, const std::vector<uint8_t>& content) = 0;

	/**
	 * Creates a new named blob. If the blob already exists, it's overwritten.
	 */
	virtual void CreateNamedBlob(const UTF8String& name, const boost::filesystem::path& sourcePath) = 0;
	
	/**
	 * Gets a blob by address.
	 * \exception BlobReadException The blob with the given address couldn't be read, e.g. it doesn't exist or a permissions failure.
	 */
	virtual std::vector<uint8_t> GetBlob(const Address& address) const = 0;

	/**
	 * Saves the blob store settings to the given property tree
	 */
	virtual void SaveSettings(boost::property_tree::ptree& ptree) const = 0;
private:
	
};

}
}
}

