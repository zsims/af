#pragma once

#include "bslib/blob/BlobStore.hpp"

#include <boost/filesystem/path.hpp>

#include <string>

namespace af {
namespace bslib {
namespace blob {

/**
 * Manages blobs with-in a directory
 */
class DirectoryBlobStore : public BlobStore
{
public:
	static const std::string TYPE;
	explicit DirectoryBlobStore(const boost::filesystem::path& rootPath);
	explicit DirectoryBlobStore(const boost::property_tree::ptree& settings);
	DirectoryBlobStore(const Uuid& id, const boost::property_tree::ptree& settings);
	Uuid GetId() const override { return _id; }
	UTF8String GetTypeString() const override { return TYPE; }
	void CreateBlob(const Address& address, const std::vector<uint8_t>& content) override;
	void CreateNamedBlob(const UTF8String& name, const boost::filesystem::path& sourcePath) override;
	std::vector<uint8_t> GetBlob(const Address& address) const override;
	boost::property_tree::ptree ConvertToPropertyTree() const override;
private:
	const boost::filesystem::path _rootPath;
	const Uuid _id;
};

}
}
}
