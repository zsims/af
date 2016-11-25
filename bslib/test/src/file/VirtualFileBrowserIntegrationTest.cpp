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

TEST_F(VirtualFileBrowserIntegrationTest, List_Success)
{
	// Arrange
	const auto path = GetUniqueExtendedTempPath().EnsureTrailingSlash();
	const auto fooPath = (path / "Foo").EnsureTrailingSlash();
	fs::CreateDirectories(fooPath);
	const auto barPath = (fooPath / "Bar").EnsureTrailingSlash();
	fs::CreateDirectories(barPath);
	const auto samsonPath = fooPath / "samson.txt";
	const auto sakoPath = barPath / "sako.txt";
	WriteFile(samsonPath, "samson was here");
	const auto sakoContentAddress = WriteFile(sakoPath, "sako was here");
	_adder->Add(path.ToString());
	// Recreate "samson.txt", and note this should be tracked as completely new file
	fs::Remove(samsonPath);
	const auto fileAddress = WriteFile(samsonPath, "samson was here with some new content");
	// Also delete bar
	fs::RemoveAll(barPath);
	// Add a new top level directory
	const auto fizzPath = (path / "fizz").EnsureTrailingSlash();
	fs::CreateDirectories(fizzPath);
	_adder->Add(path.ToString());

	// Act
	const auto browser = _uow->CreateVirtualFileBrowser();
	const auto files = browser->List(0, 100);

	// Assert
	{
		const auto matched = std::any_of(files.begin(), files.end(), [&](const VirtualFile& file) {
			return file.fullPath == path && file.type == FileType::Directory;
		});
		EXPECT_TRUE(matched) << "find " << path.ToString();
	}
	{
		const auto matched = std::any_of(files.begin(), files.end(), [&](const VirtualFile& file) {
			return file.fullPath == barPath;
		});
		EXPECT_FALSE(matched) << "not find " << barPath.ToString();
	}
	{
		const auto matched = std::any_of(files.begin(), files.end(), [&](const VirtualFile& file) {
			return file.fullPath == samsonPath && file.type == FileType::RegularFile;
		});
		EXPECT_TRUE(matched) << "find " << samsonPath.ToString();
	}
	{
		const auto matched = std::any_of(files.begin(), files.end(), [&](const VirtualFile& file) {
			return file.fullPath == sakoPath;
		});
		EXPECT_FALSE(matched) << "not find " << sakoPath.ToString();
	}
	{
		const auto matched = std::any_of(files.begin(), files.end(), [&](const VirtualFile& file) {
			return file.fullPath == fizzPath && file.type == FileType::Directory;
		});
		EXPECT_TRUE(matched) << "find " << fizzPath.ToString();
	}
}


}
}
}
}
