#include "bslib/exceptions.hpp"
#include "bslib/Forest.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib/file/exceptions.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace test {

class ForestIntegrationTest : public testing::Test
{
protected:
	ForestIntegrationTest()
		: _directoryBlobStorePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
		, _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
	{
		boost::filesystem::create_directories(_directoryBlobStorePath);
		auto blobStore = std::make_unique<blob::DirectoryBlobStore>(_directoryBlobStorePath.string());
		_forest.reset(new Forest(_forestDbPath.string(), std::move(blobStore)));
	}

	~ForestIntegrationTest()
	{
		_forest.reset();

		boost::system::error_code rec;
		boost::filesystem::remove_all(_directoryBlobStorePath, rec);

		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
	}

	void CreateFile(const boost::filesystem::path& path)
	{
		std::ofstream f(path.string(), std::ofstream::out | std::ofstream::app);
	}

	const boost::filesystem::path _directoryBlobStorePath;
	const boost::filesystem::path _forestDbPath;
	std::unique_ptr<Forest> _forest;
};

TEST_F(ForestIntegrationTest, CreateSuccess)
{
	// Arrange
	// Act
	_forest->Create();
	// Assert
	EXPECT_TRUE(boost::filesystem::exists(_forestDbPath.string()));
}

TEST_F(ForestIntegrationTest, CreateThrowsIfExists)
{
	// Arrange
	CreateFile(_forestDbPath);

	// Act
	// Assert
	EXPECT_THROW(_forest->Create(), DatabaseAlreadyExistsException);
}

TEST_F(ForestIntegrationTest, OpenThrowsIfNotExists)
{
	// Arrange
	// Act
	// Assert
	EXPECT_THROW(_forest->Open(), DatabaseNotFoundException);
}

TEST_F(ForestIntegrationTest, UnitOfWorkCommit)
{
	// Arrange
	_forest->Create();

	ObjectAddress objectAddress;
	{
		auto uow = _forest->CreateUnitOfWork();
		const std::vector<uint8_t> content = { 1, 2, 3, 4 };
		const auto blobAddress = uow->CreateBlob(content);
		objectAddress = uow->CreateFileObject("/here/it/is", blobAddress);
		// Act
		uow->Commit();
	}

	// Assert
	{
		auto uow = _forest->CreateUnitOfWork();
		EXPECT_EQ(objectAddress, uow->GetFileObject(objectAddress).address);
	}
}

TEST_F(ForestIntegrationTest, CreateUnitOfWorkTwiceFails)
{
	// Arrange
	_forest->Create();

	// Act
	// Assert
	// No support for multiple connections yet, so this isn't possible
	auto uow1 = _forest->CreateUnitOfWork();
	EXPECT_THROW(_forest->CreateUnitOfWork(), std::runtime_error);
}

TEST_F(ForestIntegrationTest, UnitOfWorkImplicitRollback)
{
	// Arrange
	_forest->Create();

	// Act
	ObjectAddress objectAddress;
	{
		auto uow = _forest->CreateUnitOfWork();

		const std::vector<uint8_t> content = { 1, 2, 3, 4 };
		const auto blobAddress = uow->CreateBlob(content);
		objectAddress = uow->CreateFileObject("/somewhere", blobAddress);
	}

	// Assert
	{
		auto uow = _forest->CreateUnitOfWork();
		EXPECT_THROW(uow->GetFileObject(objectAddress), file::ObjectNotFoundException);
	}
}

TEST_F(ForestIntegrationTest, CreateBlobDuplicateSuccess)
{
	// Arrange
	_forest->Create();
	auto uow = _forest->CreateUnitOfWork();
	const std::vector<uint8_t> content = {1, 2, 3, 4};

	// Act
	const auto blobAddress = uow->CreateBlob(content);
	const auto blobAddress2 = uow->CreateBlob(content);

	// Assert
	EXPECT_EQ(blobAddress, blobAddress2);
}

TEST_F(ForestIntegrationTest, Stuff)
{
	_forest->Create();

	auto uow = _forest->CreateUnitOfWork();
	const std::vector<uint8_t> content = {1, 2, 3, 4};
	const auto blobAddress = uow->CreateBlob(content);
	const auto objectAddress = uow->CreateFileObject("/xyz", blobAddress);

	// WOW!
	const auto storedObject = uow->GetFileObject(objectAddress);
	EXPECT_EQ(objectAddress, storedObject.address);
	EXPECT_EQ("/xyz", storedObject.fullPath);
	EXPECT_EQ(blobAddress, storedObject.contentBlobAddress);

	uow->Commit();
}

}
}
}