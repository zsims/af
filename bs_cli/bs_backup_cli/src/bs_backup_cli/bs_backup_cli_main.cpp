#include <bslib/Backup.hpp>
#include <bslib/blob/BlobStoreManager.hpp>
#include <bslib/default_locations.hpp>
#include <bslib/exceptions.hpp>
#include <bslib/file/FileAdder.hpp>
#include <bslib/file/exceptions.hpp>
#include <bslib/blob/DirectoryBlobStore.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <memory>

namespace {

int Backup(const af::bslib::UTF8String& sourcePath, const boost::filesystem::path& targetDirectoryPath)
{
	try
	{
		const auto defaultDbPath = af::bslib::GetDefaultBackupDatabasePath();
		if (defaultDbPath.empty())
		{
			std::cerr << "Failed to determine the location of the backup database" << std::endl;
			return -1;
		}
		std::cout << "Using the backup database from " << defaultDbPath << std::endl;
		boost::filesystem::create_directories(defaultDbPath.parent_path());
		af::bslib::blob::BlobStoreManager blobStoreManager(af::bslib::GetDefaultStoreSettingsPath());
		blobStoreManager.AddBlobStore(std::make_shared<af::bslib::blob::DirectoryBlobStore>(targetDirectoryPath));
		af::bslib::Backup backup(defaultDbPath, "CLI", blobStoreManager);
		backup.OpenOrCreate();

		auto uow = backup.CreateUnitOfWork();
		auto runRecorder = uow->CreateFileBackupRunRecorder();
		const auto runId = runRecorder->Start();
		auto adder = uow->CreateFileAdder(runId);

		adder->GetEventManager().Subscribe([](const auto& fileEvent) {
			std::cout << fileEvent.action << " " << fileEvent.fullPath.ToString() << std::endl;
		});

		adder->Add(sourcePath);
		runRecorder->Stop(runId);
		uow->Commit();
		return 0;
	}
	catch (const af::bslib::file::PathNotFoundException& e)
	{
		std::cerr << e.what() << std::endl;
		return 2;
	}
	catch (const af::bslib::file::SourcePathNotSupportedException& e)
	{
		std::cerr << e.what() << std::endl;
		return 3;
	}
	catch (const af::bslib::CreateDatabaseFailedException& e)
	{
		std::cerr << e.what() << std::endl;
		return 4;
	}
	catch (const af::bslib::DatabaseNotFoundException & e)
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
	std::wstring targetDirectoryPath;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "print usage message")
		("source,s", po::wvalue(&sourcePath)->required(), "Source path to backup (file or directory)")
		("target,t", po::wvalue(&targetDirectoryPath)->required(), "Target directory to save the backup to");

	po::positional_options_description positionalOptions;
	positionalOptions.add("source", 1);

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

	return Backup(af::bslib::WideToUTF8String(sourcePath), targetDirectoryPath);
}
