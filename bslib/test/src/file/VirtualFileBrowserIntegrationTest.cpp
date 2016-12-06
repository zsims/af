#include "bslib/file/VirtualFileBrowser.hpp"
#include "bslib/file/fs/operations.hpp"
#include "bslib_test_util/TestBase.hpp"
#include "bslib_test_util/gtest_boost_filesystem_fix.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <algorithm>

namespace af {
namespace bslib {
namespace file {
namespace test {

class VirtualFileBrowserIntegrationTest : public bslib_test_util::TestBase
{
protected:
	VirtualFileBrowserIntegrationTest()
	{
		_testBackup.Create();
		_uow = _testBackup.GetBackup().CreateUnitOfWork();
		_adder = _uow->CreateFileAdder(Uuid::Create());
	}

	std::unique_ptr<UnitOfWork> _uow;
	std::unique_ptr<FileAdder> _adder;
};

TEST_F(VirtualFileBrowserIntegrationTest, ListRoots_Success)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	const auto fooPath = (path / "Foo").EnsureTrailingSlash();
	fs::CreateDirectories(fooPath);
	_adder->Add(path.ToString());
	const auto root = path.GetIntermediatePaths()[0];

	// Act
	const auto browser = _uow->CreateVirtualFileBrowser();
	const auto roots = browser->ListRoots(0, 100);

	// Assert
	ASSERT_EQ(1, roots.size());
	{
		const auto matched = std::any_of(roots.begin(), roots.end(), [&](const VirtualFile& file) {
			// TODO: as the root doesn't match an event it doesn't have a known type
			return file.fullPath == root && file.type == FileType::Unknown;
		});
		EXPECT_TRUE(matched) << "find " << path.ToString();
	}
}


}
}
}
}
