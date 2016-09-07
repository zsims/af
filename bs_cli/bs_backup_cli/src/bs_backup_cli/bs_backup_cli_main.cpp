#include <bslib/Forest.hpp>
#include <bslib/exceptions.hpp>
#include <bslib/file/FileAdderEs.hpp>
#include <bslib/file/exceptions.hpp>
#include <bslib/blob/DirectoryBlobStore.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>

#include <iostream>
#include <string>
#include <memory>

namespace {

int Backup(const boost::filesystem::path& sourcePath, const boost::filesystem::path& targetDirectoryPath)
{
	const auto forestDb = targetDirectoryPath / "backup.fdb";

	try
	{
		af::bslib::Forest forest(forestDb, std::make_unique<af::bslib::blob::DirectoryBlobStore>(targetDirectoryPath));
		forest.OpenOrCreate();

		auto uow = forest.CreateUnitOfWork();
		auto adder = uow->CreateFileAdderEs();

		adder->SubscribeToEmit([](const auto& fileEvent) {
			std::cout << fileEvent.action << " " << fileEvent.fullPath << std::endl;
		});

		adder->Add(sourcePath);
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
}

}

int main(int argc, char* argv[])
{
	namespace po = boost::program_options;

	std::string sourcePath;
	std::string targetDirectoryPath;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "print usage message")
		("source,s", po::value(&sourcePath)->required(), "Source path to backup (file or directory)")
		("target,t", po::value(&targetDirectoryPath)->required(), "Target directory to save the backup to");

	po::positional_options_description positionalOptions;
	positionalOptions.add("source", 1);

	po::variables_map vm;
	try
	{
		po::command_line_parser parser(argc, argv);
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

	return Backup(sourcePath, targetDirectoryPath);
}
