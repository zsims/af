#include "bslib/forest.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/object/FileObjectInfoRepository.hpp"
#include "bslib/object/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace object {
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

	const FileObjectInfo objectInfo1(ObjectAddress("5323df2207d99a74fbe169e3eba035e635779792"), "/foo", blobInfo1.GetAddress());
	const FileObjectInfo objectInfo2(ObjectAddress("f5979d9f79727592759272503253405739475393"), "/foo", blobInfo2.GetAddress());
	const FileObjectInfo objectInfo3(ObjectAddress("5793273948759387987921653297557398753498"), "/bar/here", blobInfo1.GetAddress());

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
	ASSERT_THROW(repo.AddObject(objectInfo2), DuplicateObjectException);
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
