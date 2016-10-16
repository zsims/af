#pragma once

#include <boost/filesystem/path.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace af {
namespace test_util {

class TestEnvironment : public testing::Environment
{
public:
	virtual void SetUp();
	virtual void TearDown();
	static void SetTemporaryDirectory(const boost::filesystem::path& path);
	static boost::filesystem::path GetTemporaryDirectory();
private:
	static boost::filesystem::path _temporaryDirectory;
};

}
}

