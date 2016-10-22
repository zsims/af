#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/file/fs/path.hpp"
#include "bslib/unicode.hpp"
#include "bslib_test_util/TestBackup.hpp"

#include <boost/filesystem/path.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vector>

namespace af {
namespace bslib_test_util {

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
	bslib::file::fs::NativePath GetUniqueExtendedTempPath() const;

	/**
	 * Creates a file with the given content at the given path
	 * \returns The blob address of the content
	 */
	bslib::blob::Address WriteFile(const boost::filesystem::path& path, const bslib::UTF8String& content = "");
	bslib::blob::Address WriteFile(const bslib::file::fs::NativePath& path, const bslib::UTF8String& content = "");
private:
	const boost::filesystem::path _testTemporaryPath;
protected:
	TestBackup _testBackup;
};

}
}
