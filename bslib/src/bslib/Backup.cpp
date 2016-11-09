#include "bslib/Backup.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobStoreManager.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/exceptions.hpp"
#include "bslib/BackupDatabase.hpp"

#include <boost/filesystem.hpp>

#include <memory>

namespace af {
namespace bslib {

Backup::Backup(const boost::filesystem::path& databasePath, const UTF8String& name, const blob::BlobStoreManager& blobStoreManager)
	: _databasePath(databasePath)
	, _name(name)
	, _blobStoreManager(blobStoreManager)
{
}

Backup::~Backup()
{
	// Needed to delete incomplete types
}

void Backup::Open()
{
	_backupDatabase = std::make_unique<bslib::BackupDatabase>(_databasePath);
	_backupDatabase->Open();
}

void Backup::Create()
{
	_backupDatabase = std::make_unique<bslib::BackupDatabase>(_databasePath);
	_backupDatabase->Create();
}

void Backup::OpenOrCreate()
{
	_backupDatabase = std::make_unique<bslib::BackupDatabase>(_databasePath);
	_backupDatabase->OpenOrCreate();
}

std::unique_ptr<UnitOfWork> Backup::CreateUnitOfWork()
{
	// TODO: add support for multiple stores
	const auto stores = _blobStoreManager.GetStores();
	// TODO: throw if there's no stores
	return _backupDatabase->CreateUnitOfWork(*(stores.begin()));
}

void Backup::SaveDatabaseCopy()
{
	const auto tempPath = boost::filesystem::unique_path();
	_backupDatabase->SaveAs(tempPath);

	const auto stores = _blobStoreManager.GetStores();
	for (auto store : stores)
	{
		store->CreateNamedBlob(_name + ".db", tempPath);
	}
	boost::system::error_code ec;
	boost::filesystem::remove(tempPath, ec);
}

}
}

