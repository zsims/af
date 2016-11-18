#pragma once

#include "bslib/blob/BlobStore.hpp"

#include <boost/filesystem/path.hpp>

#include <stdexcept>
#include <string>

namespace af {
namespace bslib {
namespace blob {

class DirectoryBlobCreationFailed : public BlobStoreError
{
public:
	explicit DirectoryBlobCreationFailed(
		const std::string& msg,
		const boost::filesystem::path& rootPath,
		boost::system::error_code ec)
		: BlobStoreError(msg)
		, path(rootPath)
		, ec(ec)
	{
	}

	const boost::filesystem::path path;
	const boost::system::error_code ec;
};

/**
 * Manages blobs with-in a directory
 */
class DirectoryBlobStore : public BlobStore
{
public:
	static const std::string TYPE;
	explicit DirectoryBlobStore(const boost::filesystem::path& rootPath);
	DirectoryBlobStore(const Uuid& id, const nlohmann::json& settings);
	Uuid GetId() const override { return _id; }
	UTF8String GetTypeString() const override { return TYPE; }
	void CreateBlob(const Address& address, const std::vector<uint8_t>& content) override;
	void CreateNamedBlob(const UTF8String& name, const boost::filesystem::path& sourcePath) override;
	std::vector<uint8_t> GetBlob(const Address& address) const override;
	nlohmann::json ConvertToJson() const override;
private:
	explicit DirectoryBlobStore(const Uuid& id, const boost::filesystem::path& rootPath);

	const boost::filesystem::path _rootPath;
	const Uuid _id;
};

}
}
}
