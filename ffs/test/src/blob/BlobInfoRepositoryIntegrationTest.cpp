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

	const BlobInfo blobInfo1({ 756, 22, 44, 5, 253 }, 3573975UL);
	const BlobInfo blobInfo2({ 53, 2234, 444, 55, 32353 }, 75UL);
	const BlobInfo blobInfo3({ 7756, 522, 244, 55, 3253 }, 444UL);

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

	const BlobAddress key = {1, 2, 3, 4, 5};
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
