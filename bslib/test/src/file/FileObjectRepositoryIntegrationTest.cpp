#include "bslib/forest.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/FileObjectRepository.hpp"
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

class FileObjectRepositoryIntegrationTest : public testing::Test
{
protected:
	FileObjectRepositoryIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
	{
		Forest forest(_forestDbPath.string(), std::make_unique<blob::NullBlobStore>());
		forest.Create();

		_connection = std::make_unique<sqlitepp::ScopedSqlite3Object>();
		sqlitepp::open_database_or_throw(_forestDbPath.string().c_str(), *_connection, SQLITE_OPEN_READWRITE);
	}

	~FileObjectRepositoryIntegrationTest()
	{
		_connection.reset();
		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
	}

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	const boost::filesystem::path _forestDbPath;
};

TEST_F(FileObjectRepositoryIntegrationTest, GetAllObjects)
{
	// Arrange
	FileObjectRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);

	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(BlobAddress("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const FileObject parentObject1(ObjectAddress("9323df2207d99a74fbe169e3eba035e635779792"), "/foo", boost::none);
	const FileObject parentObject2(ObjectAddress("1323df2207d99a74fbe169e3eba035e635779792"), "/foo", boost::none, parentObject1.address);
	const FileObject objectInfo1(ObjectAddress("5323df2207d99a74fbe169e3eba035e635779792"), "/foo", blobInfo1.GetAddress());
	const FileObject objectInfo2(ObjectAddress("f5979d9f79727592759272503253405739475393"), "/foo", blobInfo2.GetAddress(), parentObject1.address);
	const FileObject objectInfo3(ObjectAddress("5793273948759387987921653297557398753498"), "/bar/here", blobInfo1.GetAddress(), parentObject2.address);
	const FileObject objectInfo4(ObjectAddress("6793273948759387987921653297557398753498"), "/bar/here/xx", boost::none);

	repo.AddObject(parentObject1);
	repo.AddObject(parentObject2);
	repo.AddObject(objectInfo1);
	repo.AddObject(objectInfo2);
	repo.AddObject(objectInfo3);
	repo.AddObject(objectInfo4);

	// Act
	const auto& result = repo.GetAllObjects();

	// Assert
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo1)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo2)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo3)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo4)));
}

TEST_F(FileObjectRepositoryIntegrationTest, GetAllObjectsByParentAddress)
{
	// Arrange
	FileObjectRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);

	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	const blob::BlobInfo blobInfo2(BlobAddress("2f59225215937593795395739753973973593571"), 157UL);
	blobRepo.AddBlob(blobInfo1);
	blobRepo.AddBlob(blobInfo2);

	const FileObject parentObject1(ObjectAddress("9323df2207d99a74fbe169e3eba035e635779792"), "/foo", boost::none);
	const FileObject parentObject2(ObjectAddress("1323df2207d99a74fbe169e3eba035e635779792"), "/foo", boost::none, parentObject1.address);
	const FileObject objectInfo1(ObjectAddress("5323df2207d99a74fbe169e3eba035e635779792"), "/foo", blobInfo1.GetAddress());
	const FileObject objectInfo2(ObjectAddress("f5979d9f79727592759272503253405739475393"), "/foo", blobInfo2.GetAddress(), parentObject1.address);
	const FileObject objectInfo3(ObjectAddress("5793273948759387987921653297557398753498"), "/bar/here", blobInfo1.GetAddress(), parentObject2.address);
	const FileObject objectInfo4(ObjectAddress("6793273948759387987921653297557398753498"), "/bar/here/xx", boost::none);

	repo.AddObject(parentObject1);
	repo.AddObject(parentObject2);
	repo.AddObject(objectInfo1);
	repo.AddObject(objectInfo2);
	repo.AddObject(objectInfo3);
	repo.AddObject(objectInfo4);

	// Act
	const auto& result = repo.GetAllObjectsByParentAddress(parentObject1.address);

	// Assert
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(objectInfo2)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(parentObject2)));
	EXPECT_THAT(result, ::testing::Not(::testing::Contains(::testing::Pointee(objectInfo3))));
	EXPECT_THAT(result, ::testing::Not(::testing::Contains(::testing::Pointee(objectInfo4))));
}

TEST_F(FileObjectRepositoryIntegrationTest, GetObject)
{
	// Arrange
	FileObjectRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const FileObject objectInfo(ObjectAddress("5793273948759387987921653297557398753498"), "/hi/phil", blobInfo1.GetAddress());
	repo.AddObject(objectInfo);

	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.address), objectInfo);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectNoBlob)
{
	// Arrange
	FileObjectRepository repo(*_connection);

	const FileObject objectInfo(ObjectAddress("8793273948759387987921653297557398753498"), "/look/phil/no/hands", boost::none);
	repo.AddObject(objectInfo);

	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.address), objectInfo);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectMissingBlobThrows)
{
	// Arrange
	FileObjectRepository repo(*_connection);

	const BlobAddress madeUpBlobAddress("2259225215937593725395732753973973593571");
	const FileObject objectInfo(ObjectAddress("8793273948759387987921653297557398753498"), "/look/phil/no/hands", madeUpBlobAddress);

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject(objectInfo), AddObjectFailedException);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectMissingParentThrows)
{
	// Arrange
	FileObjectRepository repo(*_connection);

	const ObjectAddress madeUpParentAddress("1259225215937593795395739753973973593571");
	const FileObject objectInfo(ObjectAddress("8793273948759387987921653297557398753498"), "/look/phil/no/hands", boost::none, madeUpParentAddress);

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject(objectInfo), AddObjectFailedException);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectParent)
{
	// Arrange
	FileObjectRepository repo(*_connection);

	const FileObject parentObject(ObjectAddress("9793273948759387987921653297557398753498"), "/look/phil/no/hands", boost::none);
	const FileObject objectInfo(ObjectAddress("8793273948759387987921653297557398753498"), "/look/phil/no/hands", boost::none, parentObject.address);
	repo.AddObject(parentObject);
	repo.AddObject(objectInfo);

	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.address), objectInfo);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectThrowsOnDuplicate)
{
	// Arrange
	FileObjectRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const ObjectAddress key("7323df2207d99a74fbe169e3eba035e635779721");
	const FileObject objectInfo1(key, "type", blobInfo1.GetAddress());
	const FileObject objectInfo2(key, "type", blobInfo1.GetAddress());
	repo.AddObject(objectInfo1);

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject(objectInfo2), AddObjectFailedException);
}

TEST_F(FileObjectRepositoryIntegrationTest, GetObjectThrowsOnNotFound)
{
	// Arrange
	FileObjectRepository repo(*_connection);

	const ObjectAddress key("7323df2207d99a74fbe169e3eba035e635779721");

	// Act
	// Assert
	ASSERT_THROW(repo.GetObject(key), ObjectNotFoundException);
}

}
}
}
}
