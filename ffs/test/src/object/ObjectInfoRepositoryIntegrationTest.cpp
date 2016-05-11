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

	const ObjectInfo objectInfo1(ObjectAddress("5323df2207d99a74fbe169e3eba035e635779792"), "file", {});
	const ObjectInfo objectInfo2(ObjectAddress("f5979d9f79727592759272503253405739475393"), "content", {});
	const ObjectInfo objectInfo3(ObjectAddress("5793273948759387987921653297557398753498"), "file", {});

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

	const ObjectAddress key("7323df2207d99a74fbe169e3eba035e635779721");
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
