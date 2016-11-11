#include "bslib/blob/BlobStoreManager.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib_test_util/TestBase.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace blob {
namespace test {

class BlobStoreManagerIntegrationTest : public bslib_test_util::TestBase
{
};

TEST_F(BlobStoreManagerIntegrationTest, SaveLoad_Success)
{
	// Arrange
	const auto settingsPath = GetUniqueTempPath();
	BlobStoreManager manager(settingsPath);
	manager.AddBlobStore(std::make_shared<DirectoryBlobStore>("some path"));
	manager.AddBlobStore(std::make_shared<DirectoryBlobStore>("some other path"));

	// Act
	manager.SaveToSettingsFile();

	// Assert
	{
		BlobStoreManager other(settingsPath);
		other.LoadFromSettingsFile();
		const auto& loadedStores = other.GetStores();
		ASSERT_EQ(2, loadedStores.size());
	}
}

TEST_F(BlobStoreManagerIntegrationTest, SaveToSettingsFile_CreatesPath)
{
	// Arrange
	const auto settingsPath = GetUniqueTempPath() / "deep" / "settings.xml";
	BlobStoreManager manager(settingsPath);
	manager.AddBlobStore(std::make_shared<DirectoryBlobStore>("some path"));

	// Act
	manager.SaveToSettingsFile();

	// Assert
	EXPECT_TRUE(boost::filesystem::exists(settingsPath));
}

TEST_F(BlobStoreManagerIntegrationTest, Save_OverwritesExistingSettings)
{
	// Arrange
	const auto settingsPath = GetUniqueTempPath();
	BlobStoreManager manager(settingsPath);
	manager.SaveToSettingsFile();
	ASSERT_TRUE(boost::filesystem::exists(settingsPath));

	// Act
	manager.AddBlobStore(std::make_shared<DirectoryBlobStore>("some path"));
	manager.SaveToSettingsFile();

	// Assert
	{
		BlobStoreManager other(settingsPath);
		other.LoadFromSettingsFile();
		const auto& loadedStores = other.GetStores();
		ASSERT_EQ(1, loadedStores.size());
	}
}

TEST_F(BlobStoreManagerIntegrationTest, RemoveById_Success)
{
	// Arrange
	const auto settingsPath = GetUniqueTempPath();
	BlobStoreManager manager(settingsPath);
	const auto& store1 = manager.AddBlobStore(std::make_shared<DirectoryBlobStore>("some path"));
	const auto& store2 = manager.AddBlobStore(std::make_shared<DirectoryBlobStore>("some other path"));

	// Act
	manager.RemoveById(store1.GetId());

	// Assert
	const auto& stores = manager.GetStores();
	ASSERT_EQ(1, stores.size());
}

TEST_F(BlobStoreManagerIntegrationTest, RemoveById_NotExistSuccess)
{
	// Arrange
	const auto settingsPath = GetUniqueTempPath();
	BlobStoreManager manager(settingsPath);
	const auto& store1 = manager.AddBlobStore(std::make_shared<DirectoryBlobStore>("some path"));
	const auto& store2 = manager.AddBlobStore(std::make_shared<DirectoryBlobStore>("some other path"));

	// Act
	manager.RemoveById(Uuid::Create());

	// Assert
	const auto& stores = manager.GetStores();
	ASSERT_EQ(2, stores.size());
}

TEST_F(BlobStoreManagerIntegrationTest, AddBlobStore_ThrowsOnInvalidType)
{
	// Arrange
	const auto settingsPath = GetUniqueTempPath();
	BlobStoreManager manager(settingsPath);
	boost::property_tree::ptree pt;

	// Act
	// Assert
	EXPECT_THROW(manager.AddBlobStore("not real", pt), InvalidBlobStoreTypeException);
}

}
}
}
}
