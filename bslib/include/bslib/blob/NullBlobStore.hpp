#pragma once

#include "bslib/blob/BlobStore.hpp"

namespace af {
namespace bslib {
namespace blob {

/**
 * Null implementation of the blob store where blobs are not stored.
 */
class NullBlobStore : public BlobStore
{
public:
	UTF8String GetTypeString() const override { return "null"; }
	Uuid GetId() const override { return Uuid::Create(); }
	void CreateBlob(const Address& address, const std::vector<uint8_t>& content) override { }
	void CreateNamedBlob(const UTF8String& name, const boost::filesystem::path& sourcePath) override { }
	std::vector<uint8_t> GetBlob(const Address& address) const override { return std::vector<uint8_t>(); }
	void SaveSettings(boost::property_tree::ptree& ptree) const override { }
};

}
}
}
