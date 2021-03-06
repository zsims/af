#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace blob {
namespace test {

class BlobInfoRepositoryIntegrationTest : public bslib_test_util::TestBase
{
protected:
	BlobInfoRepositoryIntegrationTest()
	{
		_testBackup.Create();
		_connection = _testBackup.ConnectToDatabase();
	}

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
};

TEST_F(BlobInfoRepositoryIntegrationTest, GetAllBlobs)
{
	// Arrange
	BlobInfoRepository repo(*_connection);

	const BlobInfo blobInfo1(Address("cf23df2207d99a74fbe169e3eba035e633b65d94"), 3573975UL);
	const BlobInfo blobInfo2(Address("5323df2207d99a74fbe169e3eba035e635779792"), 75UL);
	const BlobInfo blobInfo3(Address("f259225215937593795395739753973973593571"), 444UL);

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

	const Address key("cf23df2207d99a74fbe169e3eba035e633b65d94");
	const BlobInfo blobInfo1(key, 3573975UL);
	const BlobInfo blobInfo2(key, 7UL);
	repo.AddBlob(blobInfo1);

	// Act
	// Assert
	ASSERT_THROW(repo.AddBlob(blobInfo2), DuplicateBlobException);
}

TEST_F(BlobInfoRepositoryIntegrationTest, FindBlobNullIfNotFound)
{
	// Arrange
	BlobInfoRepository repo(*_connection);

	const Address key("cf23df2207d99a74fbe169e3eba035e633b65d94");
	const Address key2("ef23df2207d99a74fbe169e3eba035e633b65d94");
	const BlobInfo blobInfo1(key, 3573975UL);
	repo.AddBlob(blobInfo1);

	// Act
	const auto result = repo.FindBlob(key2);

	// Assert
	EXPECT_FALSE(result);
}

TEST_F(BlobInfoRepositoryIntegrationTest, FindBlobSuccess)
{
	// Arrange
	BlobInfoRepository repo(*_connection);

	const BlobInfo blobInfo1(Address("cf23df2207d99a74fbe169e3eba035e633b65d94"), 3573975UL);
	const BlobInfo blobInfo2(Address("5323df2207d99a74fbe169e3eba035e635779792"), 75UL);

	repo.AddBlob(blobInfo1);
	repo.AddBlob(blobInfo2);

	// Act
	const auto result = repo.FindBlob(blobInfo1.GetAddress());

	// Assert
	ASSERT_TRUE(result);
	EXPECT_EQ(blobInfo1, *result);
}

}
}
}
}
