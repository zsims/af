#include "utility/TestEnvironment.hpp"

#include <boost/filesystem.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	//testing::GTEST_FLAG(filter) = "SuiteName.TestName";
	testing::InitGoogleMock(&argc, argv);

	auto tempDirectory = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	af::bslib::test::utility::TestEnvironment::SetTemporaryDirectory(af::bslib::file::fs::NativePath(tempDirectory.string()));
	std::cout << "Using " << tempDirectory << " for temporary test files" << std::endl;
	testing::AddGlobalTestEnvironment(new af::bslib::test::utility::TestEnvironment());

	return RUN_ALL_TESTS();
}

