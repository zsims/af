#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace blob {
namespace test {

class DirectoryBlobStoreIntegrationTest : public bslib_test_util::TestBase
{
};


TEST_F(DirectoryBlobStoreIntegrationTest, SaveLoad)
{
	// Arrange
	const auto path = GetUniqueTempPath();
	boost::filesystem::create_directories(path);
	DirectoryBlobStore store(path);

	const std::vector<uint8_t> content = {
		1, 2, 3, 4, 4, 5, 3, 2, 1
	};
	const auto address = Address::CalculateFromContent(content);

	// Act
	store.CreateBlob(address, content);

	// Assert
	const auto actualBlobContent = store.GetBlob(address);
	EXPECT_EQ(content, actualBlobContent);
}

TEST_F(DirectoryBlobStoreIntegrationTest, GetBlobThrowsIfNotExist)
{
	// Arrange
	const auto path = GetUniqueTempPath();
	boost::filesystem::create_directories(path);
	DirectoryBlobStore store(path);

	const std::vector<uint8_t> content = {
		1, 2, 3, 4, 4, 5, 3, 2, 1
	};

	const Address address("1234a123451234b123451234a123451234b12345");

	// Act
	EXPECT_THROW(store.GetBlob(address), BlobReadException);
}

}
}
}
}
