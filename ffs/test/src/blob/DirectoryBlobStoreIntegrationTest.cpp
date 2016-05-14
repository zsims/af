#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/blob/DirectoryBlobStore.hpp"
#include "ffs/blob/exceptions.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace ffs {
namespace blob {
namespace test {

class DirectoryBlobStoreIntegrationTest : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		_storagePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
		boost::filesystem::create_directories(_storagePath);
	}

	virtual void TearDown() override
	{
		boost::filesystem::remove_all(_storagePath);
	}

	boost::filesystem::path _storagePath;
};


TEST_F(DirectoryBlobStoreIntegrationTest, SaveLoad)
{
	// Arrange
	auto repo = std::make_shared<BlobInfoRepository>();
	DirectoryBlobStore store(repo, _storagePath.string());

	const std::vector<uint8_t> content = {
		1, 2, 3, 4, 4, 5, 3, 2, 1
	};

	// Act
	const auto blobAddress = store.CreateBlob(content);

	// Assert
	const auto actualBlobContent = store.GetBlob(blobAddress);
	EXPECT_EQ(content, actualBlobContent);
}

TEST_F(DirectoryBlobStoreIntegrationTest, GetBlobThrowsIfNotExist)
{
	// Arrange
	auto repo = std::make_shared<BlobInfoRepository>();
	DirectoryBlobStore store(repo, _storagePath.string());

	const std::vector<uint8_t> content = {
		1, 2, 3, 4, 4, 5, 3, 2, 1
	};

	const BlobAddress address("1234a123451234b123451234a123451234b12345");

	// Act
	EXPECT_THROW(store.GetBlob(address), BlobReadException);
}

}
}
}
}
