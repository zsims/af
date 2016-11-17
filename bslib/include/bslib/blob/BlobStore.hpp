#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/unicode.hpp"
#include "bslib/Uuid.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>

#include <memory>
#include <stdexcept>
#include <vector>

namespace af {
namespace bslib {
namespace blob {

class BlobStoreError : public std::runtime_error
{
public:
	explicit BlobStoreError(const std::string& msg)
		: std::runtime_error(msg)
	{
	}
};

class CreateBlobFailed : public BlobStoreError
{
public:
	explicit CreateBlobFailed(
		const std::string& msg,
		const boost::filesystem::path& path,
		boost::system::error_code ec = {})
		: BlobStoreError(msg)
		, path(path)
	{
	}

	const boost::filesystem::path path;
	const boost::system::error_code ec;
};

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
	 * Returns the blob store settings as a property tree
	 */
	virtual boost::property_tree::ptree ConvertToPropertyTree() const = 0;
};

}
}
}

