#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ffs/Forest.hpp"
#include "ffs/blob/DirectoryBlobStore.hpp"
#include "ffs/blob/exceptions.hpp"

namespace af {
namespace ffs {
namespace test {

TEST(ForestIntegrationTest, Opens)
{
	Forest forest("test.db");
	blob::DirectoryBlobStore directoryStore(forest.GetBlobInfoRepository(), "C:\\here");

	const std::vector<uint8_t> content = {1, 2, 3, 4};
	const auto blobAddress = directoryStore.CreateBlob(content);

	// forest.CreateObject(address);

}

}
}
}
