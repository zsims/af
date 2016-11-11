#include "bslib/blob/BlobStoreManager.hpp"

#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/blob/exceptions.hpp"

#include <boost/filesystem.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <algorithm>
#include <memory>

namespace af {
namespace bslib {
namespace blob {

namespace bpt = boost::property_tree;

BlobStoreManager::BlobStoreManager(const boost::filesystem::path& settingsPath)
	: _settingsPath(settingsPath)
{
}

void BlobStoreManager::LoadFromSettingsFile()
{
	std::unique_lock<std::mutex> lock(_mutex);
	_stores.clear();
	bpt::ptree pt;
	bpt::read_xml(_settingsPath.string(), pt);
	for (const auto& store : pt.get_child("stores"))
	{
		AddBlobStoreNoLock(store.first, store.second);
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

	bpt::ptree pt;
	bpt::ptree stores;
	for (auto& store : _stores)
	{
		bpt::ptree storeSettings;
		store->SaveSettings(storeSettings);
		stores.add_child(store->GetTypeString(), storeSettings);
	}
	pt.add_child("stores", stores);

	auto writerSettings = boost::property_tree::xml_writer_make_settings<std::string>('\t', 1, "utf-8");
	bpt::write_xml(_settingsPath.string(), pt, std::locale(), writerSettings);
}

BlobStore& BlobStoreManager::AddBlobStore(std::shared_ptr<BlobStore> store)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_stores.push_back(std::move(store));
	return *_stores.back();
}

BlobStore& BlobStoreManager::AddBlobStore(const UTF8String& typeString, const boost::property_tree::ptree& settingsChunk)
{
	std::unique_lock<std::mutex> lock(_mutex);
	return AddBlobStoreNoLock(typeString, settingsChunk);
}

BlobStore& BlobStoreManager::AddBlobStoreNoLock(const UTF8String& typeString, const boost::property_tree::ptree& settingsChunk)
{
	if (typeString == "directory")
	{
		_stores.push_back(std::make_shared<DirectoryBlobStore>(Uuid::Create(), settingsChunk));
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
