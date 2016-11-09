#include <bslib/Backup.hpp>
#include <bslib/blob/BlobStoreManager.hpp>
#include <bslib/default_locations.hpp>
#include <bslib/exceptions.hpp>
#include <bslib/file/FileFinder.hpp>
#include <bslib/file/FileRestorer.hpp>
#include <bslib/file/exceptions.hpp>
#include <bslib/blob/DirectoryBlobStore.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <memory>

namespace {

int Restore(const af::bslib::UTF8String& pathToRestore, const af::bslib::UTF8String& targetPath, const boost::filesystem::path& backupSource)
{
	auto queryPath = af::bslib::file::fs::NativePath(pathToRestore);
	queryPath.MakePreferred();

	try
	{
		const auto defaultDbPath = af::bslib::GetDefaultBackupDatabasePath();
		if (defaultDbPath.empty())
		{
			std::cerr << "Failed to determine the location of the backup database" << std::endl;
			return -1;
		}
		boost::filesystem::create_directories(defaultDbPath.parent_path());
		std::cout << "Using the backup database from " << defaultDbPath << std::endl;
		af::bslib::blob::BlobStoreManager blobStoreManager;
		blobStoreManager.AddBlobStore(std::make_unique<af::bslib::blob::DirectoryBlobStore>(backupSource));
		af::bslib::Backup backup(defaultDbPath, "CLI", blobStoreManager);
		backup.OpenOrCreate();

		auto uow = backup.CreateUnitOfWork();
		const auto finder = uow->CreateFileFinder();
		auto restorer = uow->CreateFileRestorer();
		const auto events = finder->GetLastChangedEventsStartingWithPath(queryPath);

		restorer->GetEventManager().Subscribe([](const auto& fileRestoreEvent) {
			std::cout << fileRestoreEvent.action << " " << fileRestoreEvent.originalEvent.fullPath.ToString() << " to " << fileRestoreEvent.targetPath.ToString() << std::endl;
		});
		restorer->Restore(events, targetPath);
		return 0;
	}
	catch (const af::bslib::file::PathNotFoundException& e)
	{
		std::cerr << e.what() << std::endl;
		return 2;
	}
	catch (const af::bslib::file::TargetPathNotSupportedException& e)
	{
		std::cerr << e.what() << std::endl;
		return 3;
	}
	catch (const af::bslib::CreateDatabaseFailedException& e)
	{
		std::cerr << e.what() << std::endl;
		return 4;
	}
	catch (const af::bslib::DatabaseNotFoundException& e)
	{
		std::cerr << e.what() << std::endl;
		return 5;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return -1;
	}
	catch (...)
	{
		std::cerr << "Unexpected error" << std::endl;
		return -1;
	}
}

}

// Per https://msdn.microsoft.com/en-us/library/bky3b5dh.aspx
// Support Unicode arguments
int wmain(int argc, wchar_t *argv[], wchar_t *envp[])
{
	namespace po = boost::program_options;

	std::wstring sourcePath;
	std::wstring pathToRestore;
	std::wstring destinationPath;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "print usage message")
		("path,p", po::wvalue(&pathToRestore)->required(), "Path to restore")
		("source,s", po::wvalue(&sourcePath)->required(), "Source to restore from")
		("destination,d", po::wvalue(&destinationPath)->required(), "Destination path to restore to");

	po::positional_options_description positionalOptions;
	positionalOptions.add("path", 1);

	po::variables_map vm;
	try
	{
		po::wcommand_line_parser parser(argc, argv);
		parser.options(desc).positional(positionalOptions);
		po::store(parser.run(), vm);

		if (vm.count("help"))
		{
			std::cout << desc << std::endl;
			return 0;
		}

		// throw if any arguments conflict or are missing
		po::notify(vm);
	}
	catch (const po::required_option& e)
	{
		std::cerr << "Option " << e.get_option_name() << " is required" << std::endl;
		return 1;
	}
	catch (const po::error& e)
	{
		std::cerr << "Error processing command line arguments: " << e.what() << std::endl;
		return 1;
	}

	return Restore(af::bslib::WideToUTF8String(pathToRestore), af::bslib::WideToUTF8String(destinationPath), boost::filesystem::path(sourcePath));
}
