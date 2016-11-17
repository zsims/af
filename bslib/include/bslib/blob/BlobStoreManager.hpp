#pragma once

#include "bslib/blob/BlobStore.hpp"

#include <boost/filesystem/path.hpp>

#include <string>
#include <memory>
#include <mutex>

namespace af {
namespace bslib {
namespace blob {

class BlobStoreManager
{
public:
	explicit BlobStoreManager(const boost::filesystem::path& settingsPath);

	/**
	 * Loads blob stores from the configured settings path
	 */
	void LoadFromSettingsFile();

	/**
	 * Persists the blob stores to the configured settings path, overwriting it if it exists.
	 * Any intermediate directories are created if the path doesn't exist.
	 */
	void SaveToSettingsFile() const;

	/**
	 * Adds a blob store to be managed by this manager.
	 */
	BlobStore& AddBlobStore(std::shared_ptr<BlobStore> store);

	/**
	 * Adds a blob store from a given type string and associated settings
	 */
	BlobStore& AddBlobStore(const UTF8String& typeString, const nlohmann::json& settings);

	/**
	 * Removes the blob store identified by the given id (if any)
	 */
	void RemoveById(const Uuid& id);

	const std::vector<std::shared_ptr<BlobStore>> GetStores() const;
private:
	BlobStore& BlobStoreManager::AddBlobStoreNoLock(const UTF8String& typeString, const nlohmann::json& settings);
	const boost::filesystem::path _settingsPath;
	mutable std::mutex _mutex;
	std::vector<std::shared_ptr<BlobStore>> _stores;
};

}
}
}