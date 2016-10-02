#include "bslib/exceptions.hpp"
#include "bslib/Forest.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileAdder.hpp"
#include "bslib/file/fs/operations.hpp"
#include "TestBase.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace test {

class ForestIntegrationTest : public TestBase
{
protected:
	void CreateFile(const boost::filesystem::path& path)
	{
		std::ofstream f(path.string(), std::ofstream::out | std::ofstream::app);
	}
};

TEST_F(ForestIntegrationTest, CreateSuccess)
{
	// Arrange
	// Act
	_testForest.CreateWithNullStore();
	// Assert
	EXPECT_TRUE(boost::filesystem::exists(_testForest.GetForestDbPath()));
}

TEST_F(ForestIntegrationTest, CreateThrowsIfExists)
{
	// Arrange
	CreateFile(_testForest.GetForestDbPath());

	// Act
	// Assert
	EXPECT_THROW(_testForest.Create(), DatabaseAlreadyExistsException);
}

TEST_F(ForestIntegrationTest, OpenThrowsIfNotExists)
{
	// Arrange
	// Act
	// Assert
	EXPECT_THROW(_testForest.Open(), DatabaseNotFoundException);
}

TEST_F(ForestIntegrationTest, UnitOfWorkCommit)
{
	// Arrange
	_testForest.CreateWithNullStore();
	auto& forest = _testForest.GetForest();

	const auto targetPath = file::fs::GenerateUniqueTempPath().EnsureTrailingSlash();
	{
		auto uow = forest.CreateUnitOfWork();
		auto adder = uow->CreateFileAdder();
		file::fs::CreateDirectories(targetPath);
		adder->Add(targetPath.ToString());
		// Act
		uow->Commit();
	}

	// Assert
	{
		auto uow = forest.CreateUnitOfWork();
		auto finder = uow->CreateFileFinder();
		EXPECT_TRUE(finder->FindLastChangedEventByPath(targetPath));
	}
}

TEST_F(ForestIntegrationTest, CreateUnitOfWorkTwiceFails)
{
	// Arrange
	_testForest.Create();
	auto& forest = _testForest.GetForest();

	// Act
	// Assert
	// No support for multiple connections yet, so this isn't possible
	auto uow1 = forest.CreateUnitOfWork();
	EXPECT_THROW(forest.CreateUnitOfWork(), std::runtime_error);
}

TEST_F(ForestIntegrationTest, UnitOfWorkImplicitRollback)
{
	// Arrange
	_testForest.Create();
	auto& forest = _testForest.GetForest();

	// Act
	const auto targetPath = file::fs::GenerateUniqueTempPath().EnsureTrailingSlash();
	{
		auto uow = forest.CreateUnitOfWork();
		auto adder = uow->CreateFileAdder();
		file::fs::CreateDirectories(targetPath);
		adder->Add(targetPath.ToString());
	}

	// Assert
	{
		auto uow = forest.CreateUnitOfWork();
		auto finder = uow->CreateFileFinder();
		EXPECT_FALSE(finder->FindLastChangedEventByPath(targetPath));
	}
}

TEST_F(ForestIntegrationTest, OpenOrCreateExisting)
{
	// Arrange
	_testForest.Create();
	_testForest.Close();
	Forest secondForest(_testForest.GetForestDbPath(), std::make_unique<blob::NullBlobStore>());

	// Act
	// Assert
	ASSERT_NO_THROW(secondForest.OpenOrCreate());
}

TEST_F(ForestIntegrationTest, OpenOrCreateNew)
{
	// Arrange
	// Act
	// Assert
	ASSERT_NO_THROW(_testForest.OpenOrCreate());
}

}
}
}
