#include "bslib/Backup.hpp"

#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/NullBlobStore.hpp"
#include "bslib/exceptions.hpp"
#include "bslib/BackupDatabase.hpp"

#include <boost/filesystem.hpp>

#ifdef WIN32
#include <shlobj.h>
#include <shlwapi.h>
#endif

#include <memory>

namespace af {
namespace bslib {

boost::filesystem::path GetDefaultBackupDatabasePath()
{
	PWSTR buffer = nullptr;
	const auto result = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &buffer);
	const std::unique_ptr<WCHAR, std::function<void(PWSTR)>> autoBuffer(buffer, [](PWSTR value) { CoTaskMemFree(value); });
	if (result != S_OK)
	{
		return boost::filesystem::path();
	}
	return boost::filesystem::path(autoBuffer.get()) / "af" / "backup.db";
}

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

void Backup::SaveDatabaseCopy()
{
	const auto tempPath = boost::filesystem::unique_path();
	_backupDatabase->SaveAs(tempPath);
	_blobStore->CreateNamedBlob(_name + ".db", tempPath);
	boost::system::error_code ec;
	boost::filesystem::remove(tempPath, ec);
}

}
}

