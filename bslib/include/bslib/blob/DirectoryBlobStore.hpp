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
	explicit DirectoryBlobStore(const boost::filesystem::path& rootPath);
	explicit DirectoryBlobStore(const boost::property_tree::ptree& settings);
	Uuid GetId() const override { return _id; }
	UTF8String GetTypeString() const override { return "directory"; }
	void CreateBlob(const Address& address, const std::vector<uint8_t>& content) override;
	void CreateNamedBlob(const UTF8String& name, const boost::filesystem::path& sourcePath) override;
	std::vector<uint8_t> GetBlob(const Address& address) const override;
	void SaveSettings(boost::property_tree::ptree& ptree) const override;
private:
	const boost::filesystem::path _rootPath;
	const Uuid _id;
};

}
}
}
