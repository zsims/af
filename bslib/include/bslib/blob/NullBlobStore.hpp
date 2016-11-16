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
	static const std::string TYPE;

	NullBlobStore()
		: _id(Uuid::Create())
	{
	}

	explicit NullBlobStore(const Uuid& id)
		: _id(id)
	{
	}
	UTF8String GetTypeString() const override { return TYPE; }
	Uuid GetId() const override { return _id; }
	void CreateBlob(const Address& address, const std::vector<uint8_t>& content) override { }
	void CreateNamedBlob(const UTF8String& name, const boost::filesystem::path& sourcePath) override { }
	std::vector<uint8_t> GetBlob(const Address& address) const override { return std::vector<uint8_t>(); }
	boost::property_tree::ptree ConvertToPropertyTree() const override
	{
		boost::property_tree::ptree result;
		result.add("id", _id.ToString());
		return result;
	}
private:
	const Uuid _id;
};

}
}
}
