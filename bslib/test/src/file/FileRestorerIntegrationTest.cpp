#include "bslib/forest.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "utility/gtest_boost_filesystem_fix.hpp"
#include "utility/matchers.hpp"
#include "utility/ScopedExclusiveFileAccess.hpp"

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileRestorerIntegrationTest : public testing::Test
{
protected:
	FileRestorerIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
		, _targetPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
		, _restorePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
	{
		boost::filesystem::create_directories(_targetPath);
		auto blobStore = std::make_unique<blob::DirectoryBlobStore>(_targetPath);
		_forest.reset(new Forest(_forestDbPath.string(), std::move(blobStore)));
		_forest->Create();

		_uow = _forest->CreateUnitOfWork();
		_adder = _uow->CreateFileAdder();
		_restorer = _uow->CreateFileRestorer();
		_finder = _uow->CreateFileFinder();
	}

	~FileRestorerIntegrationTest()
	{
		_adder.reset();
		_uow.reset();
		_forest.reset();

		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
		boost::filesystem::remove_all(_targetPath, ec);
	}

	void CreateFile(const boost::filesystem::path& path, const std::string& content)
	{
		std::ofstream f(path.string(), std::ofstream::out);
		f << content;
	}

	const boost::filesystem::path _forestDbPath;
	const boost::filesystem::path _targetPath;
	const boost::filesystem::path _restorePath;
	std::unique_ptr<Forest> _forest;
	std::unique_ptr<UnitOfWork> _uow;
	std::unique_ptr<FileAdder> _adder;
	std::unique_ptr<FileRestorer> _restorer;
	std::unique_ptr<FileFinder> _finder;
};

TEST_F(FileRestorerIntegrationTest, RestoreSingle_FileToFilePath)
{
	// Arrange
	const auto address = _adder->Add("/here", { 1, 2, 3 });

	// Act
	_restorer->RestoreSingle(address, _restorePath);

	// Assert
	const auto& restored = _restorer->GetRestoredPaths();
	const auto& skipped = _restorer->GetSkippedPaths();
	EXPECT_THAT(restored, ::testing::Contains(_restorePath));
	EXPECT_THAT(skipped, ::testing::Not(::testing::Contains(_restorePath)));
}

TEST_F(FileRestorerIntegrationTest, RestoreTree_FileToFilePath)
{
	// Arrange
	const auto address = _adder->Add("/here", { 1, 2, 3 });

	// Act
	_restorer->RestoreTree(address, _restorePath);

	// Assert
	const auto& restored = _restorer->GetRestoredPaths();
	const auto& skipped = _restorer->GetSkippedPaths();
	EXPECT_THAT(restored, ::testing::Contains(_restorePath));
	EXPECT_THAT(skipped, ::testing::Not(::testing::Contains(_restorePath)));
}

TEST_F(FileRestorerIntegrationTest, RestoreSingle_FileToExistingDirectory)
{
	// Arrange
	boost::filesystem::create_directories(_restorePath);
	const auto address = _adder->Add("/here", { 1, 2, 3 });
	const auto expectedPath = _restorePath / "here";

	// Act
	_restorer->RestoreSingle(address, _restorePath);

	// Assert
	const auto& restored = _restorer->GetRestoredPaths();
	const auto& skipped = _restorer->GetSkippedPaths();
	EXPECT_THAT(restored, ::testing::Contains(expectedPath));
	EXPECT_THAT(skipped, ::testing::Not(::testing::Contains(expectedPath)));
}

TEST_F(FileRestorerIntegrationTest, RestoreTree_FileToExistingDirectory)
{
	// Arrange
	boost::filesystem::create_directories(_restorePath);
	const auto address = _adder->Add("/here", { 1, 2, 3 });
	const auto expectedPath = _restorePath / "here";

	// Act
	_restorer->RestoreTree(address, _restorePath);

	// Assert
	const auto& restored = _restorer->GetRestoredPaths();
	const auto& skipped = _restorer->GetSkippedPaths();
	EXPECT_THAT(restored, ::testing::Contains(expectedPath));
	EXPECT_THAT(skipped, ::testing::Not(::testing::Contains(expectedPath)));
}

TEST_F(FileRestorerIntegrationTest, RestoreTree_DirectoryToExistingDirectory)
{
	// Arrange
	const auto path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	boost::filesystem::create_directories(path);
	auto deepDirectory = path / boost::filesystem::unique_path();
	boost::filesystem::create_directories(deepDirectory);
	const auto filePath = path / "file.dat";
	const auto deepFilePath = deepDirectory / "deep.dat";
	CreateFile(filePath, "hello");
	CreateFile(deepFilePath, "hell");
	_adder->Add(path);

	boost::filesystem::create_directories(_restorePath);
	const auto expectedDeepDirectoryPath = _restorePath / path.filename() / deepDirectory.filename();
	const auto expectedDeepFilePath = _restorePath / path.filename() / deepDirectory.filename() / "deep.dat";
	const auto expectedFilePath = _restorePath / path.filename() / "file.dat";

	// Act
	const auto& rootAddress = _finder->FindReference(path);
	ASSERT_TRUE(rootAddress);
	_restorer->RestoreTree(rootAddress.value().fileObjectAddress, _restorePath);

	// Assert
	const auto& restored = _restorer->GetRestoredPaths();
	const auto& skipped = _restorer->GetSkippedPaths();
	EXPECT_THAT(restored, ::testing::Contains(expectedFilePath));
	EXPECT_THAT(restored, ::testing::Contains(expectedDeepFilePath));
	EXPECT_THAT(restored, ::testing::Contains(expectedDeepDirectoryPath));
	EXPECT_THAT(expectedDeepFilePath, HasSameFileContents(deepFilePath));
	EXPECT_THAT(expectedFilePath, HasSameFileContents(filePath));
}

TEST_F(FileRestorerIntegrationTest, RestoreTree_DirectoryToNonExistingDirectory)
{
	// Arrange
	const auto path = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	boost::filesystem::create_directories(path);
	auto deepDirectory = path / boost::filesystem::unique_path();
	boost::filesystem::create_directories(deepDirectory);
	const auto filePath = path / "file.dat";
	const auto deepFilePath = deepDirectory / "deep.dat";
	CreateFile(filePath, "hello");
	CreateFile(deepFilePath, "hell");
	_adder->Add(path);

	const auto expectedDeepDirectoryPath = _restorePath / deepDirectory.filename();
	const auto expectedDeepFilePath = _restorePath / deepDirectory.filename() / "deep.dat";
	const auto expectedFilePath = _restorePath / "file.dat";

	// Act
	const auto& rootAddress = _finder->FindReference(path);
	ASSERT_TRUE(rootAddress);
	_restorer->RestoreTree(rootAddress.value().fileObjectAddress, _restorePath);

	// Assert
	const auto& restored = _restorer->GetRestoredPaths();
	const auto& skipped = _restorer->GetSkippedPaths();
	EXPECT_THAT(restored, ::testing::Contains(expectedFilePath));
	EXPECT_THAT(restored, ::testing::Contains(expectedDeepFilePath));
	EXPECT_THAT(restored, ::testing::Contains(expectedDeepDirectoryPath));
	EXPECT_THAT(expectedDeepFilePath, HasSameFileContents(deepFilePath));
	EXPECT_THAT(expectedFilePath, HasSameFileContents(filePath));
}

}
}
}
}
