#include "bslib/forest.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/FileRefRepository.hpp"
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

class FileRefRepositoryIntegrationTest : public testing::Test
{
protected:
	FileRefRepositoryIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
	{
		Forest forest(_forestDbPath.string(), std::make_unique<blob::NullBlobStore>());
		forest.Create();

		_connection = std::make_unique<sqlitepp::ScopedSqlite3Object>();
		sqlitepp::open_database_or_throw(_forestDbPath.string().c_str(), *_connection, SQLITE_OPEN_READWRITE);
	}

	~FileRefRepositoryIntegrationTest()
	{
		_connection.reset();
		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
	}

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	const boost::filesystem::path _forestDbPath;
};

TEST_F(FileRefRepositoryIntegrationTest, GetReference)
{
	// Arrange
	FileRefRepository repo(*_connection);
	FileObjectRepository fileRepo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const auto id = fileRepo.AddObject("/hi/phil", blobInfo1.GetAddress());
	const auto reference = repo.SetGetReference("/hi/phil", id);

	// Act
	// Assert
	EXPECT_EQ(repo.GetReference(reference.fullPath), reference);
}

TEST_F(FileRefRepositoryIntegrationTest, AddReferenceMissingObjectThrows)
{
	// Arrange
	FileRefRepository repo(*_connection);
	// Act
	// Assert
	ASSERT_THROW(repo.SetReference("/look/phil", 69), AddRefFailedException);
}

TEST_F(FileRefRepositoryIntegrationTest, AddReferenceOverwritesOnDuplicate)
{
	// Arrange
	FileRefRepository repo(*_connection);
	FileObjectRepository fileRepo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const std::string path("/same");
	const auto id1 = fileRepo.AddObject(path, boost::none);
	const auto id2 = fileRepo.AddObject(path, blobInfo1.GetAddress());

	const auto reference = repo.SetGetReference(path, id1);
	EXPECT_EQ(repo.GetReference(path), reference);

	// Act
	const auto updatedReference = repo.SetGetReference(path, id1);

	// Assert
	ASSERT_EQ(repo.GetReference(path), updatedReference);
}

TEST_F(FileRefRepositoryIntegrationTest, GetReferenceThrowsOnNotFound)
{
	// Arrange
	FileRefRepository repo(*_connection);
	// Act
	// Assert
	ASSERT_THROW(repo.GetReference("uhoh"), RefNotFoundException);
}

}
}
}
}
