#pragma once

#include "bslib/file/fs/path.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace af {
namespace bslib {
namespace test {
namespace utility {

class TestEnvironment : public testing::Environment
{
public:
	virtual void SetUp();
	virtual void TearDown();
	static void SetTemporaryDirectory(const file::fs::NativePath& path);
	static file::fs::NativePath GetTemporaryDirectory();
private:
	static file::fs::NativePath _temporaryDirectory;
};

}
}
}
}

