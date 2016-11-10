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
	/**
	 * Loads blob stores from the given file path
	 */
	void LoadFromSettingsFile(const boost::filesystem::path& settingsPath);

	/**
	 * Persists the blob stores to the given path, overwriting it if it exists.
	 */
	void SaveToSettingsFile(const boost::filesystem::path& settingsPath) const;

	/**
	 * Adds a blob store to be managed by this manager.
	 */
	BlobStore& AddBlobStore(std::shared_ptr<BlobStore> store);

	/**
	 * Adds a blob store from a given type string and associated settings
	 */
	BlobStore& AddBlobStore(const UTF8String& typeString, const boost::property_tree::ptree& settingsChunk);

	/**
	 * Removes the blob store identified by the given id (if any)
	 */
	void RemoveById(const Uuid& id);

	const std::vector<std::shared_ptr<BlobStore>> GetStores() const;
private:
	BlobStore& BlobStoreManager::AddBlobStoreNoLock(const UTF8String& typeString, const boost::property_tree::ptree& settingsChunk);
	mutable std::mutex _mutex;
	std::vector<std::shared_ptr<BlobStore>> _stores;
};

}
}
}