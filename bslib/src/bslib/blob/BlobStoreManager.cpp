#include "bslib/blob/BlobStoreManager.hpp"

#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/blob/exceptions.hpp"

#include <boost/filesystem.hpp>
#include <json.hpp>

#include <algorithm>
#include <fstream>
#include <memory>

namespace af {
namespace bslib {
namespace blob {

BlobStoreManager::BlobStoreManager(const boost::filesystem::path& settingsPath)
	: _settingsPath(settingsPath)
{
}

void BlobStoreManager::LoadFromSettingsFile()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_stores.clear();

	std::ifstream f;
	f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	f.open(_settingsPath.string(), std::ios::in | std::ifstream::binary);
	const auto settings = nlohmann::json::parse(f);
	if (settings.at("stores").is_array())
	{
		for (const auto& store : settings["stores"])
		{
			AddBlobStoreNoLock(store.at("type").get<std::string>(), store.at("settings"));
		}
	}
}

void BlobStoreManager::SaveToSettingsFile() const
{
	std::unique_lock<std::mutex> lock(_mutex);

	const auto& parentPath = _settingsPath.parent_path();
	if (!boost::filesystem::exists(parentPath))
	{
		boost::filesystem::create_directories(parentPath);
	}

	nlohmann::json stores;
	for (auto& store : _stores)
	{
		nlohmann::json storeEntry;
		storeEntry["id"] = store->GetId().ToString();
		storeEntry["type"] = store->GetTypeString();
		storeEntry["settings"] = store->ConvertToJson();
		stores.push_back(storeEntry);
	}

	std::ofstream f;
	f.exceptions(std::ofstream::failbit | std::ofstream::badbit);
	f.open(_settingsPath.string(), std::ios::out | std::ofstream::binary);
	nlohmann::json settings;
	settings["stores"] = stores;
	f << std::setw(2) << settings;
}

BlobStore& BlobStoreManager::AddBlobStore(std::shared_ptr<BlobStore> store)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_stores.push_back(std::move(store));
	return *_stores.back();
}

BlobStore& BlobStoreManager::AddBlobStore(const UTF8String& typeString, const nlohmann::json& settings)
{
	std::unique_lock<std::mutex> lock(_mutex);
	return AddBlobStoreNoLock(typeString, settings);
}

BlobStore& BlobStoreManager::AddBlobStoreNoLock(const UTF8String& typeString, const nlohmann::json& settings)
{
	if (typeString == DirectoryBlobStore::TYPE)
	{
		_stores.push_back(std::make_shared<DirectoryBlobStore>(Uuid::Create(), settings));
	}
	else if (typeString == NullBlobStore::TYPE)
	{
		_stores.push_back(std::make_shared<NullBlobStore>(Uuid::Create()));
	}
	else
	{
		throw InvalidBlobStoreTypeException(typeString + " is not a valid blob store type");
	}
	return *_stores.back();
}

void BlobStoreManager::RemoveById(const Uuid& id)
{
	std::unique_lock<std::mutex> lock(_mutex);
	auto newEnd = std::remove_if(_stores.begin(), _stores.end(), [&](const auto& s) {
		return s->GetId() == id;
	});
	_stores.erase(newEnd, _stores.end());
}

const std::vector<std::shared_ptr<BlobStore>> BlobStoreManager::GetStores() const
{
	std::unique_lock<std::mutex> lock(_mutex);
	return _stores;
}

}
}
}
