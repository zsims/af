#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/file/fs/path.hpp"
#include "bslib/unicode.hpp"
#include "utility/TestBackup.hpp"

#include <boost/filesystem/path.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

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

	/**
	 * Creates a a file with the given content at the given path
	 * \returns The blob address of the content
	 */
	blob::Address CreateFile(const boost::filesystem::path& path, const UTF8String& content = "");
	blob::Address CreateFile(const file::fs::NativePath& path, const UTF8String& content = "");
private:
	const boost::filesystem::path _testTemporaryPath;
protected:
	utility::TestBackup _testBackupDatabase;
};

}
}
}
