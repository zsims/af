#include "ffs/exceptions.hpp"
#include "ffs/Forest.hpp"
#include "ffs/blob/DirectoryBlobStore.hpp"
#include "ffs/blob/exceptions.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace ffs {
namespace test {

class ForestIntegrationTest : public testing::Test
{
protected:
	virtual void SetUp() override
	{
		_directoryBlobStorePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
		boost::filesystem::create_directories(_directoryBlobStorePath);
		_forestDbPath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb");
		_forest.reset(new Forest(_forestDbPath.string()));
	}

	virtual void TearDown() override
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

	boost::filesystem::path _directoryBlobStorePath;
	boost::filesystem::path _forestDbPath;
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

TEST_F(ForestIntegrationTest, Stuff)
{
	const auto storagePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	boost::filesystem::create_directories(storagePath);

	_forest->Create();
	blob::DirectoryBlobStore directoryStore(_forest->GetBlobInfoRepository(), storagePath.string());

	const std::vector<uint8_t> content = {1, 2, 3, 4};
	const auto blobAddress = directoryStore.CreateBlob(content);

	const object::ObjectBlobList objectBlobs = {
		{"content", blobAddress}
	};
	const auto objectAddress = _forest->CreateObject("file", objectBlobs);

	// WOW!
	const auto storedObject = _forest->GetObject(objectAddress);
	EXPECT_EQ(objectAddress, storedObject.GetAddress());
	EXPECT_EQ("file", storedObject.GetType());
	EXPECT_EQ(objectBlobs, storedObject.GetBlobs());
}

}
}
}
