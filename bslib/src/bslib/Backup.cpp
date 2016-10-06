#include "bslib/Backup.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/exceptions.hpp"
#include "bslib/BackupDatabase.hpp"

#include <boost/filesystem.hpp>

namespace af {
namespace bslib {

Backup::Backup(const boost::filesystem::path& databasePath, const UTF8String& name)
	: _databasePath(databasePath)
	, _name(name)
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
	return _backupDatabase->CreateUnitOfWork(*_blobStore);
}

void Backup::AddBlobStore(std::unique_ptr<blob::BlobStore> blobStore)
{
	_blobStore = std::move(blobStore);
}

}
}

