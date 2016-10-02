#pragma once

#include "bslib/file/fs/path.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace af {
namespace bslib {
namespace test {

std::string GenerateUuid();

class TestBase : public testing::Test
{
protected:
	TestBase();
	virtual ~TestBase();

	/**
	 * Gets a unique temp path
	 */
	file::fs::NativePath GetUniqueTempPath() const;

	/**
	 * Generates a unique extended path greater than 260 characters
	 */
	file::fs::NativePath GetUniqueExtendedTempPath() const;
private:
	const file::fs::NativePath _testTemporaryPath;
};

}
}
}
