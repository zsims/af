#include "bslib/exceptions.hpp"
#include "bslib/Forest.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileAdder.hpp"
#include "bslib/file/fs/operations.hpp"

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

	const auto targetPath = file::fs::GenerateUniqueTempPath().EnsureTrailingSlash();
	{
		auto uow = _forest->CreateUnitOfWork();
		auto adder = uow->CreateFileAdder();
		file::fs::CreateDirectories(targetPath);
		adder->Add(targetPath.ToString());
		// Act
		uow->Commit();
	}

	// Assert
	{
		auto uow = _forest->CreateUnitOfWork();
		auto finder = uow->CreateFileFinder();
		EXPECT_TRUE(finder->FindLastChangedEventByPath(targetPath));
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
	const auto targetPath = file::fs::GenerateUniqueTempPath().EnsureTrailingSlash();
	{
		auto uow = _forest->CreateUnitOfWork();
		auto adder = uow->CreateFileAdder();
		file::fs::CreateDirectories(targetPath);
		adder->Add(targetPath.ToString());
	}

	// Assert
	{
		auto uow = _forest->CreateUnitOfWork();
		auto finder = uow->CreateFileFinder();
		EXPECT_FALSE(finder->FindLastChangedEventByPath(targetPath));
	}
}

TEST_F(ForestIntegrationTest, OpenOrCreateExisting)
{
	// Arrange
	_forest->Create();
	_forest.reset();
	Forest secondForest(_forestDbPath, std::make_unique<blob::NullBlobStore>());

	// Act
	// Assert
	ASSERT_NO_THROW(secondForest.OpenOrCreate());
}

TEST_F(ForestIntegrationTest, OpenOrCreateNew)
{
	// Arrange
	// Act
	// Assert
	ASSERT_NO_THROW(_forest->OpenOrCreate());
}

}
}
}
