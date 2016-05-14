#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/blob/exceptions.hpp"

namespace af {
namespace ffs {
namespace blob {
namespace test {

TEST(BlobInfoRepositoryIntegrationTest, GetAllBlobs)
{
	// Arrange
	BlobInfoRepository repo;

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

TEST(BlobInfoRepositoryIntegrationTest, AddBlobThrowsOnDuplicate)
{
	// Arrange
	BlobInfoRepository repo;

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
