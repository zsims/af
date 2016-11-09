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
	void Load(const boost::filesystem::path& settingsPath);

	/**
	 * Persists the blob stores to the given path, overwriting it if it exists.
	 */
	void Save(const boost::filesystem::path& settingsPath) const;

	/**
	 * Adds a blob store to be managed by this manager.
	 */
	BlobStore& AddBlobStore(std::unique_ptr<BlobStore> store);

	/**
	 * Removes the blob store identified by the given id (if any)
	 */
	void RemoveById(const Uuid& id);

	const std::vector<std::unique_ptr<BlobStore>>& GetStores() const { return _stores; }
private:
	mutable std::mutex _mutex;
	std::vector<std::unique_ptr<BlobStore>> _stores;
};

}
}
}