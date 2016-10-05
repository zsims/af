#pragma once

#include "bslib/file/fs/path.hpp"
#include "utility/TestBackup.hpp"

#include <boost/filesystem/path.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace af {
namespace bslib {
namespace test {

class TestBase : public testing::Test
{
protected:
	TestBase();
	virtual ~TestBase();

	/**
	 * Gets a unique temp path
	 */
	boost::filesystem::path GetUniqueTempPath() const;

	/**
	 * Generates a unique extended path greater than 260 characters
	 */
	file::fs::NativePath GetUniqueExtendedTempPath() const;
private:
	const boost::filesystem::path _testTemporaryPath;
protected:
	utility::TestBackup _testBackupDatabase;
};

}
}
}
