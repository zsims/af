#include "bslib/blob/BlobStore.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace bslib {
namespace blob {
namespace test {

class MockBlobStore : public BlobStore
{
public:
	virtual ~MockBlobStore() { }
	MOCK_METHOD2(CreateBlob, void(const Address& address, const std::vector<uint8_t>& content));
	MOCK_CONST_METHOD1(GetBlob, std::vector<uint8_t>(const Address& address));
	MOCK_METHOD2(CreateNamedBlob, void(const UTF8String& name, const boost::filesystem::path& sourcePath));
};

}
}
}
}
