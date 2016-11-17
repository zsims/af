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

TEST_F(DirectoryBlobStoreIntegrationTest, CreateBlobThrowsErrorIfFails)
{
	// Arrange
	const auto path = GetUniqueTempPath();
	boost::filesystem::create_directories(path);
	DirectoryBlobStore store(path);

	// Now nuke the underlying file before DBS can write to it
	boost::filesystem::remove_all(path);

	const std::vector<uint8_t> content = {
		1, 2, 3, 4, 4, 5, 3, 2, 1
	};
	const auto address = Address::CalculateFromContent(content);

	// Act
	// Assert
	EXPECT_THROW(store.CreateBlob(address, content), CreateBlobFailed);
}

TEST_F(DirectoryBlobStoreIntegrationTest, CreatesOwnPathIfMissing)
{
	// Arrange
	const auto path = GetUniqueTempPath();
	ASSERT_FALSE(boost::filesystem::exists(path));

	// Act
	DirectoryBlobStore store(path);

	// Assert
	EXPECT_TRUE(boost::filesystem::exists(path));
}

TEST_F(DirectoryBlobStoreIntegrationTest, CtorThrowsErrorIfPathIsNonFolder)
{
	// Arrange
	const auto path = GetUniqueTempPath();
	WriteFile(path, "hahahah");

	// Act
	// Assert
	EXPECT_THROW(DirectoryBlobStore store(path), DirectoryBlobCreationFailed);
}

TEST_F(DirectoryBlobStoreIntegrationTest, CreateNamedBlob_Success)
{
	// Arrange
	const auto path = GetUniqueTempPath();
	boost::filesystem::create_directories(path);
	DirectoryBlobStore store(path);
	const auto sourcePath = GetUniqueTempPath();
	WriteFile(sourcePath, "hi");

	// Act
	store.CreateNamedBlob("backup.db", sourcePath);

	// Assert
	EXPECT_TRUE(boost::filesystem::exists(path / "backup.db"));
}

TEST_F(DirectoryBlobStoreIntegrationTest, CreateNamedBlob_OverwriteSuccess)
{
	// Arrange
	const auto path = GetUniqueTempPath();
	boost::filesystem::create_directories(path);
	DirectoryBlobStore store(path);
	const auto sourcePath = GetUniqueTempPath();
	WriteFile(sourcePath, "hi");
	store.CreateNamedBlob("backup.db", sourcePath);

	// Act
	EXPECT_NO_THROW(store.CreateNamedBlob("backup.db", sourcePath));

	// Assert
	EXPECT_TRUE(boost::filesystem::exists(path / "backup.db"));
}

TEST_F(DirectoryBlobStoreIntegrationTest, CreateNamedBlobThrowsErrorIfFails)
{
	// Arrange
	const auto path = GetUniqueTempPath();
	boost::filesystem::create_directories(path);
	DirectoryBlobStore store(path);

	// Now nuke the underlying file before DBS can write to it
	boost::filesystem::remove_all(path);
	const auto sourcePath = GetUniqueTempPath();
	WriteFile(sourcePath, "hi");

	// Act
	// Assert
	EXPECT_THROW(store.CreateNamedBlob("backup.db", sourcePath), CreateBlobFailed);
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
