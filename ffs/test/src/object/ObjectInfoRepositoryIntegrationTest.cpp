#include "ffs/forest.hpp"
#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/object/ObjectInfoRepository.hpp"
#include "ffs/object/exceptions.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace ffs {
namespace object {
namespace test {

class ObjectInfoRepositoryIntegrationTest : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		_forestDbPath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb");
		Forest forest(_forestDbPath.string());
		forest.Create();
	}

	virtual void TearDown() override
	{
		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
	}

	boost::filesystem::path _forestDbPath;
};

TEST_F(ObjectInfoRepositoryIntegrationTest, GetAllObjects)
{
	// Arrange
	ObjectInfoRepository repo(_forestDbPath.string());

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

TEST_F(ObjectInfoRepositoryIntegrationTest, GetALlObjectsPreservesObjectBlobs)
{
	// Arrange
	ObjectInfoRepository repo(_forestDbPath.string());
	blob::BlobInfoRepository blobRepo(_forestDbPath.string());

	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(BlobAddress("2f59225215937593795395739753973973593571"), 157UL);
	const blob::BlobInfo blobInfo3(BlobAddress("3259225215937593795395739753973973593571"), 227UL);
	const blob::BlobInfo blobInfo4(BlobAddress("4259225215937593795395739753973973593571"), 327UL);
	const blob::BlobInfo blobInfo5(BlobAddress("5259225215937593795395739753973973593571"), 427UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);
	blobRepo.AddBlob(blobInfo3);
	blobRepo.AddBlob(blobInfo4);
	blobRepo.AddBlob(blobInfo5);

	const ObjectBlobList expectedBlobList1 = {
		{"p", blobInfo2.GetAddress()},
		{"e", blobInfo3.GetAddress()},
		{"p", blobInfo1.GetAddress()}
	};

	const ObjectBlobList expectedBlobList2 = {
		{"p", blobInfo4.GetAddress()},
		{"e", blobInfo5.GetAddress()},
		{"p", blobInfo1.GetAddress()},
		{"t", blobInfo2.GetAddress()},
		{"p", blobInfo3.GetAddress()},
	};

	const ObjectInfo objectInfo1(ObjectAddress("5793273948759387987921653297557398753498"), "file", expectedBlobList1);
	repo.AddObject(objectInfo1);

	const ObjectInfo objectInfo2(ObjectAddress("6693273948759387987921653297557398753498"), "other", expectedBlobList2);
	repo.AddObject(objectInfo2);

	// Act
	const auto result = repo.GetAllObjects();

	// Assert
	const auto o1it = std::find_if(result.begin(), result.end(), [&](ObjectInfoPtr o) { return *o == objectInfo1; });
	ASSERT_NE(o1it, result.end());
	EXPECT_EQ(expectedBlobList1, (*o1it)->GetBlobs());

	const auto o2it = std::find_if(result.begin(), result.end(), [&](ObjectInfoPtr o) { return *o == objectInfo2; });
	ASSERT_NE(o2it, result.end());
	EXPECT_EQ(expectedBlobList2, (*o2it)->GetBlobs());

}

TEST_F(ObjectInfoRepositoryIntegrationTest, GetObject)
{
	// Arrange
	ObjectInfoRepository repo(_forestDbPath.string());
	const ObjectInfo objectInfo(ObjectAddress("5793273948759387987921653297557398753498"), "file", {});
	repo.AddObject(objectInfo);
	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.GetAddress()), objectInfo);
}

TEST_F(ObjectInfoRepositoryIntegrationTest, GetObjectPreservesObjectBlobs)
{
	// Arrange
	ObjectInfoRepository repo(_forestDbPath.string());
	blob::BlobInfoRepository blobRepo(_forestDbPath.string());

	const blob::BlobInfo blobInfo1(BlobAddress("f259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(BlobAddress("cf59225215937593795395739753973973593571"), 57UL);
	const blob::BlobInfo blobInfo3(BlobAddress("9259225215937593795395739753973973593571"), 27UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);
	blobRepo.AddBlob(blobInfo3);

	const ObjectBlobList expectedBlobList = {
		{"p", blobInfo2.GetAddress()},
		{"e", blobInfo3.GetAddress()},
		{"p", blobInfo1.GetAddress()}
	};

	const ObjectInfo objectInfo(ObjectAddress("5793273948759387987921653297557398753498"), "file", expectedBlobList);
	repo.AddObject(objectInfo);

	// Act
	const auto object = repo.GetObject(objectInfo.GetAddress());

	// Assert
	EXPECT_EQ(expectedBlobList, object.GetBlobs());
}

TEST_F(ObjectInfoRepositoryIntegrationTest, AddObjectThrowsOnDuplicate)
{
	// Arrange
	ObjectInfoRepository repo(_forestDbPath.string());

	const ObjectAddress key("7323df2207d99a74fbe169e3eba035e635779721");
	const ObjectInfo objectInfo1(key, "type", {});
	const ObjectInfo objectInfo2(key, "type", {});
	repo.AddObject(objectInfo1);

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject(objectInfo2), DuplicateObjectException);
}

TEST_F(ObjectInfoRepositoryIntegrationTest, GetObjectThrowsOnNotFound)
{
	// Arrange
	ObjectInfoRepository repo(_forestDbPath.string());

	const ObjectAddress key("7323df2207d99a74fbe169e3eba035e635779721");

	// Act
	// Assert
	ASSERT_THROW(repo.GetObject(key), ObjectNotFoundException);
}

}
}
}
}
