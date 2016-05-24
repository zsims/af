#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/blob/exceptions.hpp"
#include "ffs/blob/NullBlobStore.hpp"
#include "ffs/forest.hpp"
#include "ffs/sqlitepp/sqlitepp.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace ffs {
namespace blob {
namespace test {

class BlobInfoRepositoryIntegrationTest : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		_forestDbPath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb");
		Forest forest(_forestDbPath.string(), std::make_shared<blob::NullBlobStore>());
		forest.Create();

		_connection = std::make_unique<sqlitepp::ScopedSqlite3Object>();
		const auto result = sqlite3_open_v2(_forestDbPath.string().c_str(), *_connection, SQLITE_OPEN_READWRITE, 0);
		ASSERT_EQ(SQLITE_OK, result);
	}

	virtual void TearDown() override
	{
		_connection.reset();
		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
	}

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	boost::filesystem::path _forestDbPath;
};

TEST_F(BlobInfoRepositoryIntegrationTest, GetAllBlobs)
{
	// Arrange
	BlobInfoRepository repo(*_connection);

	const BlobInfo blobInfo1(BlobAddress("cf23df2207d99a74fbe169e3eba035e633b65d94"), 3573975UL);
	const BlobInfo blobInfo2(BlobAddress("5323df2207d99a74fbe169e3eba035e635779792"), 75UL);
	const BlobInfo blobInfo3(BlobAddress("f259225215937593795395739753973973593571"), 444UL);

	repo.AddBlob(blobInfo1);
	repo.AddBlob(blobInfo2);
	repo.AddBlob(blobInfo3);

	// Act
	const auto result = repo.GetAllBlobs();

	// Assert
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(blobInfo1)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(blobInfo2)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(blobInfo3)));
}

TEST_F(BlobInfoRepositoryIntegrationTest, AddBlobThrowsOnDuplicate)
{
	// Arrange
	BlobInfoRepository repo(*_connection);

	const BlobAddress key("cf23df2207d99a74fbe169e3eba035e633b65d94");
	const BlobInfo blobInfo1(key, 3573975UL);
	const BlobInfo blobInfo2(key, 7UL);
	repo.AddBlob(blobInfo1);

	// Act
	// Assert
	ASSERT_THROW(repo.AddBlob(blobInfo2), DuplicateBlobException);
}

}
}
}
}
