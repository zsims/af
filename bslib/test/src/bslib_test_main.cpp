#include "bslib_test_util/TestEnvironment.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	namespace po = boost::program_options;

	//testing::GTEST_FLAG(filter) = "SuiteName.TestName";
	testing::InitGoogleMock(&argc, argv);

	const auto defaultTempPath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	std::string tempPath;
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,h", "print usage message")
		("temp,t", po::value(&tempPath)->default_value(defaultTempPath.string()), "Path to use for temporary files");

	po::variables_map vm;
	po::command_line_parser parser(argc, argv);
	parser.options(desc);
	po::store(parser.run(), vm);
	vm.notify();

	if (vm.count("help"))
	{
		std::cout << desc << std::endl;
		return 0;
	}

	af::bslib_test_util::TestEnvironment::SetTemporaryDirectory(tempPath);
	std::cout << "Using " << tempPath << " for temporary test files" << std::endl;
	testing::AddGlobalTestEnvironment(new af::bslib_test_util::TestEnvironment());

	return RUN_ALL_TESTS();
}

