#include "bslib/forest.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileObjectInfoRepositoryIntegrationTest : public testing::Test
{
protected:
	FileObjectInfoRepositoryIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
	{
		Forest forest(_forestDbPath.string(), std::make_unique<blob::NullBlobStore>());
		forest.Create();

		_connection = std::make_unique<sqlitepp::ScopedSqlite3Object>();
		sqlitepp::open_database_or_throw(_forestDbPath.string().c_str(), *_connection, SQLITE_OPEN_READWRITE);
	}

	~FileObjectInfoRepositoryIntegrationTest()
	{
		_connection.reset();
		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
	}

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	const boost::filesystem::path _forestDbPath;
};

TEST_F(FileObjectInfoRepositoryIntegrationTest, GetAllObjects)
{
	// Arrange
	FileObjectInfoRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);

	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(BlobAddress("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const FileObjectInfo parentObject1(ObjectAddress("9323df2207d99a74fbe169e3eba035e635779792"), "/foo", boost::none);
	const FileObjectInfo parentObject2(ObjectAddress("1323df2207d99a74fbe169e3eba035e635779792"), "/foo", boost::none, parentObject1.address);
	const FileObjectInfo objectInfo1(ObjectAddress("5323df2207d99a74fbe169e3eba035e635779792"), "/foo", blobInfo1.GetAddress());
	const FileObjectInfo objectInfo2(ObjectAddress("f5979d9f79727592759272503253405739475393"), "/foo", blobInfo2.GetAddress(), parentObject1.address);
	const FileObjectInfo objectInfo3(ObjectAddress("5793273948759387987921653297557398753498"), "/bar/here", blobInfo1.GetAddress(), parentObject2.address);
	const FileObjectInfo objectInfo4(ObjectAddress("6793273948759387987921653297557398753498"), "/bar/here/xx", boost::none);

	repo.AddObject(parentObject1);
	repo.AddObject(parentObject2);
	repo.AddObject(objectInfo1);
	repo.AddObject(objectInfo2);
	repo.AddObject(objectInfo3);
	repo.AddObject(objectInfo4);

	// Act
	const auto result = repo.GetAllObjects();

	// Assert
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo1)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo2)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo3)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo4)));
}

TEST_F(FileObjectInfoRepositoryIntegrationTest, GetObject)
{
	// Arrange
	FileObjectInfoRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const FileObjectInfo objectInfo(ObjectAddress("5793273948759387987921653297557398753498"), "/hi/phil", blobInfo1.GetAddress());
	repo.AddObject(objectInfo);

	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.address), objectInfo);
}

TEST_F(FileObjectInfoRepositoryIntegrationTest, AddObjectNoBlob)
{
	// Arrange
	FileObjectInfoRepository repo(*_connection);

	const FileObjectInfo objectInfo(ObjectAddress("8793273948759387987921653297557398753498"), "/look/phil/no/hands", boost::none);
	repo.AddObject(objectInfo);

	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.address), objectInfo);
}

TEST_F(FileObjectInfoRepositoryIntegrationTest, AddObjectMissingBlobThrows)
{
	// Arrange
	FileObjectInfoRepository repo(*_connection);

	const BlobAddress madeUpBlobAddress("2259225215937593725395732753973973593571");
	const FileObjectInfo objectInfo(ObjectAddress("8793273948759387987921653297557398753498"), "/look/phil/no/hands", madeUpBlobAddress);

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject(objectInfo), AddObjectFailedException);
}

TEST_F(FileObjectInfoRepositoryIntegrationTest, AddObjectMissingParentThrows)
{
	// Arrange
	FileObjectInfoRepository repo(*_connection);

	const ObjectAddress madeUpParentAddress("1259225215937593795395739753973973593571");
	const FileObjectInfo objectInfo(ObjectAddress("8793273948759387987921653297557398753498"), "/look/phil/no/hands", boost::none, madeUpParentAddress);

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject(objectInfo), AddObjectFailedException);
}

TEST_F(FileObjectInfoRepositoryIntegrationTest, AddObjectParent)
{
	// Arrange
	FileObjectInfoRepository repo(*_connection);

	const FileObjectInfo parentObject(ObjectAddress("9793273948759387987921653297557398753498"), "/look/phil/no/hands", boost::none);
	const FileObjectInfo objectInfo(ObjectAddress("8793273948759387987921653297557398753498"), "/look/phil/no/hands", boost::none, parentObject.address);
	repo.AddObject(parentObject);
	repo.AddObject(objectInfo);

	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.address), objectInfo);
}

TEST_F(FileObjectInfoRepositoryIntegrationTest, AddObjectThrowsOnDuplicate)
{
	// Arrange
	FileObjectInfoRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const ObjectAddress key("7323df2207d99a74fbe169e3eba035e635779721");
	const FileObjectInfo objectInfo1(key, "type", blobInfo1.GetAddress());
	const FileObjectInfo objectInfo2(key, "type", blobInfo1.GetAddress());
	repo.AddObject(objectInfo1);

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject(objectInfo2), AddObjectFailedException);
}

TEST_F(FileObjectInfoRepositoryIntegrationTest, GetObjectThrowsOnNotFound)
{
	// Arrange
	FileObjectInfoRepository repo(*_connection);

	const ObjectAddress key("7323df2207d99a74fbe169e3eba035e635779721");

	// Act
	// Assert
	ASSERT_THROW(repo.GetObject(key), ObjectNotFoundException);
}

}
}
}
}
