#include "bslib/forest.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/FileRefRepository.hpp"
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
	FileObjectInfoRepository fileRepo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const FileObjectInfo objectInfo(ObjectAddress("5793273948759387987921653297557398753498"), "/hi/phil", blobInfo1.GetAddress());
	fileRepo.AddObject(objectInfo);

	const FileRef reference(objectInfo.fullPath, objectInfo.address);
	repo.SetReference(reference);

	// Act
	// Assert
	EXPECT_EQ(repo.GetReference(reference.fullPath), reference);
}

TEST_F(FileRefRepositoryIntegrationTest, AddReferenceMissingObjectThrows)
{
	// Arrange
	FileRefRepository repo(*_connection);

	const ObjectAddress fileAddress("8793273948759387987921653297557398753498");
	const FileRef reference("/look/phil", fileAddress);

	// Act
	// Assert
	ASSERT_THROW(repo.SetReference(reference), AddRefFailedException);
}

TEST_F(FileRefRepositoryIntegrationTest, AddReferenceOverwritesOnDuplicate)
{
	// Arrange
	FileRefRepository repo(*_connection);
	FileObjectInfoRepository fileRepo(*_connection);
	blob::BlobInfoRepository blobRepo(*_connection);
	const blob::BlobInfo blobInfo1(BlobAddress("1259225215937593795395739753973973593571"), 444UL);
	blobRepo.AddBlob(blobInfo1);

	const std::string path("/same");
	const FileObjectInfo objectInfo(ObjectAddress("5793273948759387987921653297557398753498"), path, boost::none);
	const FileObjectInfo objectInfo2(ObjectAddress("6793273948759387987921653297557398753498"), path, blobInfo1.GetAddress());
	fileRepo.AddObject(objectInfo);
	fileRepo.AddObject(objectInfo2);

	const FileRef reference(path, objectInfo.address);
	repo.SetReference(reference);
	EXPECT_EQ(repo.GetReference(path), reference);

	// Act
	const FileRef updatedReference(path, objectInfo.address);
	repo.SetReference(reference);

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
