#include "bslib/blob/BlobStoreManager.hpp"

#include "bslib/blob/DirectoryBlobStore.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <algorithm>
#include <memory>

namespace af {
namespace bslib {
namespace blob {

namespace bpt = boost::property_tree;

void BlobStoreManager::Load(const boost::filesystem::path& settingsPath)
{
	std::unique_lock<std::mutex> lock(_mutex);
	bpt::ptree pt;
	bpt::read_xml(settingsPath.string(), pt);
	for (const auto& store : pt.get_child("stores"))
	{
		if (store.first == "directory")
		{
			_stores.push_back(std::make_unique<DirectoryBlobStore>(store.second));
		}
	}
}

void BlobStoreManager::Save(const boost::filesystem::path& settingsPath) const
{
	std::unique_lock<std::mutex> lock(_mutex);
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
	bpt::write_xml(settingsPath.string(), pt, std::locale(), writerSettings);
}

BlobStore& BlobStoreManager::AddBlobStore(std::unique_ptr<BlobStore> store)
{
	std::unique_lock<std::mutex> lock(_mutex);
	_stores.push_back(std::move(store));
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

}
}
}
