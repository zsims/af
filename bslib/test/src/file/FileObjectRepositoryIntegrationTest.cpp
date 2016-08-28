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

	const auto parent1 = repo.AddGetObject("/foo", boost::none);
	const auto parent2 = repo.AddGetObject("/foo", boost::none, parent1.id);
	const auto object1 = repo.AddGetObject("/foo", blobInfo1.GetAddress());
	const auto object2 = repo.AddGetObject("/foo", blobInfo2.GetAddress(), parent1.id);
	const auto object3 = repo.AddGetObject("/bar/here", blobInfo1.GetAddress(), parent2.id);
	const auto object4 = repo.AddGetObject("/bar/here/xx", boost::none);

	// Act
	const auto& result = repo.GetAllObjects();

	// Assert
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(object1)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(object2)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(object3)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(object4)));
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

	const auto parent1 = repo.AddGetObject("/foo", boost::none);
	const auto parent2 = repo.AddGetObject("/foo", boost::none, parent1.id);
	const auto object1 = repo.AddGetObject("/foo", blobInfo1.GetAddress());
	const auto object2 = repo.AddGetObject("/foo", blobInfo2.GetAddress(), parent1.id);
	const auto object3 = repo.AddGetObject("/bar/here", blobInfo1.GetAddress(), parent2.id);
	const auto object4 = repo.AddGetObject("/bar/here/xx", boost::none);

	// Act
	const auto& result = repo.GetAllObjectsByParentId(parent1.id);

	// Assert
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(object2)));
	EXPECT_THAT(result, ::testing::Contains(::testing::Pointee(parent2)));
	EXPECT_THAT(result, ::testing::Not(::testing::Contains(::testing::Pointee(object3))));
	EXPECT_THAT(result, ::testing::Not(::testing::Contains(::testing::Pointee(object4))));
}

TEST_F(FileObjectRepositoryIntegrationTest, GetObject)
{
	// Arrange
	FileObjectRepository repo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const auto objectInfo = repo.AddGetObject("/hi/phil", blobInfo1.GetAddress());

	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.id), objectInfo);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectNoBlob)
{
	// Arrange
	FileObjectRepository repo(*_connection);

	const auto objectInfo = repo.AddGetObject("/look/phil/no/hands", boost::none);

	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.id), objectInfo);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectMissingBlobThrows)
{
	// Arrange
	FileObjectRepository repo(*_connection);
	const BlobAddress madeUpBlobAddress("2259225215937593725395732753973973593571");

	// Act
	// Assert
	ASSERT_THROW(repo.AddObject("/look/phil/no/hands", madeUpBlobAddress), AddObjectFailedException);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectMissingParentThrows)
{
	// Arrange
	FileObjectRepository repo(*_connection);
	// Act
	// Assert
	ASSERT_THROW(repo.AddObject("/look/phil/no/hands", boost::none, 69), AddObjectFailedException);
}

TEST_F(FileObjectRepositoryIntegrationTest, AddObjectParent)
{
	// Arrange
	FileObjectRepository repo(*_connection);
	const auto parentId = repo.AddObject("/look/phil/no/hands", boost::none);
	const auto objectInfo = repo.AddGetObject("/look/phil/no/hands", boost::none, parentId);
	// Act
	// Assert
	EXPECT_EQ(repo.GetObject(objectInfo.id), objectInfo);
}

TEST_F(FileObjectRepositoryIntegrationTest, GetObjectThrowsOnNotFound)
{
	// Arrange
	FileObjectRepository repo(*_connection);

	// Act
	// Assert
	ASSERT_THROW(repo.GetObject(69), ObjectNotFoundException);
}

}
}
}
}
