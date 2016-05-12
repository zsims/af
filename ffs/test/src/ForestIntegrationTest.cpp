
#include "ffs/Forest.hpp"
#include "ffs/blob/DirectoryBlobStore.hpp"
#include "ffs/blob/exceptions.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace ffs {
namespace test {

TEST(ForestIntegrationTest, Things)
{
	// Arrange
	const auto storagePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	boost::filesystem::create_directories(storagePath);

	Forest forest;
	blob::DirectoryBlobStore directoryStore(forest.GetBlobInfoRepository(), storagePath.string());

	const std::vector<uint8_t> content = {1, 2, 3, 4};
	const auto blobAddress = directoryStore.CreateBlob(content);

	const object::ObjectBlobList objectBlobs = {
		{"content", blobAddress}
	};
	const auto objectAddress = forest.CreateObject("file", objectBlobs);

	// WOW!
	const auto storedObject = forest.GetObject(objectAddress);
	EXPECT_EQ(objectAddress, storedObject.GetAddress());
	EXPECT_EQ("file", storedObject.GetType());
	EXPECT_EQ(objectBlobs, storedObject.GetBlobs());
}

}
}
}
