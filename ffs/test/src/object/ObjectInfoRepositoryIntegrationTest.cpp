#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ffs/object/ObjectInfoRepository.hpp"
#include "ffs/object/exceptions.hpp"

namespace af {
namespace ffs {
namespace object {
namespace test {

TEST(ObjectInfoRepositoryIntegrationTest, GetAllObjects)
{
	// Arrange
	ObjectInfoRepository repo;

	const ObjectInfo objectInfo1({ 756, 22, 44, 5, 253 }, "file", {});
	const ObjectInfo objectInfo2({ 53, 2234, 444, 55, 32353 }, "content", {});
	const ObjectInfo objectInfo3({ 7756, 522, 244, 55, 3253 }, "file", {});

	repo.AddObject(objectInfo1);
	repo.AddObject(objectInfo2);
	repo.AddObject(objectInfo3);

	// Act
	const auto result = repo.GetAllObjects();

	// Assert
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo1)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo2)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo3)));
}

TEST(ObjectInfoRepositoryIntegrationTest, AddObjectThrowsOnDuplicate)
{
	// Arrange
	ObjectInfoRepository repo;

	const ObjectAddress key = {1, 2, 3, 4, 5};
	const ObjectInfo objectInfo1(key, "type", {});
	const ObjectInfo objectInfo2(key, "type", {});
	repo.AddObject(objectInfo1);

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject(objectInfo2), DuplicateObjectException);
}

}
}
}
}
