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
	BlobStoreManager manager;
	const auto settingsPath = GetUniqueTempPath();
	manager.AddBlobStore(std::make_unique<DirectoryBlobStore>("some path"));
	manager.AddBlobStore(std::make_unique<DirectoryBlobStore>("some other path"));

	// Act
	manager.Save(settingsPath);

	// Assert
	{
		BlobStoreManager other;
		other.Load(settingsPath);
		const auto& loadedStores = other.GetStores();
		ASSERT_EQ(2, loadedStores.size());
	}
}

TEST_F(BlobStoreManagerIntegrationTest, Save_OverwritesExistingSettings)
{
	// Arrange
	BlobStoreManager manager;
	const auto settingsPath = GetUniqueTempPath();
	manager.Save(settingsPath);
	ASSERT_TRUE(boost::filesystem::exists(settingsPath));

	// Act
	manager.AddBlobStore(std::make_unique<DirectoryBlobStore>("some path"));
	manager.Save(settingsPath);

	// Assert
	{
		BlobStoreManager other;
		other.Load(settingsPath);
		const auto& loadedStores = other.GetStores();
		ASSERT_EQ(1, loadedStores.size());
	}
}

TEST_F(BlobStoreManagerIntegrationTest, RemoveById_Success)
{
	// Arrange
	BlobStoreManager manager;
	const auto& store1 = manager.AddBlobStore(std::make_unique<DirectoryBlobStore>("some path"));
	const auto& store2 = manager.AddBlobStore(std::make_unique<DirectoryBlobStore>("some other path"));

	// Act
	manager.RemoveById(store1.GetId());

	// Assert
	const auto& stores = manager.GetStores();
	ASSERT_EQ(1, stores.size());
}

TEST_F(BlobStoreManagerIntegrationTest, RemoveById_NotExistSuccess)
{
	// Arrange
	BlobStoreManager manager;
	const auto& store1 = manager.AddBlobStore(std::make_unique<DirectoryBlobStore>("some path"));
	const auto& store2 = manager.AddBlobStore(std::make_unique<DirectoryBlobStore>("some other path"));

	// Act
	manager.RemoveById(Uuid::Create());

	// Assert
	const auto& stores = manager.GetStores();
	ASSERT_EQ(2, stores.size());
}

}
}
}
}
