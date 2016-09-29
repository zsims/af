#include "bslib/forest.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/file/fs/operations.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
#include "file/test_utility/ScopedWorkingDirectory.hpp"
#include "utility/gtest_boost_filesystem_fix.hpp"
#include "utility/matchers.hpp"
#include "utility/ScopedExclusiveFileAccess.hpp"

#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace test {

class FileRestorerEsIntegrationTest : public testing::Test
{
protected:
	FileRestorerEsIntegrationTest()
		: _forestDbPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%.fdb"))
		, _targetPath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
		, _restorePath(fs::GenerateUniqueTempPath())
		, _sampleBasePath(fs::GenerateUniqueTempPath())
		, _sampleSubDirectory(_sampleBasePath / "sub")
		, _sampleFilePath(_sampleBasePath / "base.dat")
		, _sampleSubFilePath(_sampleSubDirectory / "subfile.dat")
	{
		boost::filesystem::create_directories(_targetPath);
		fs::CreateDirectories(_restorePath);
		auto blobStore = std::make_unique<blob::DirectoryBlobStore>(_targetPath);
		_forest.reset(new Forest(_forestDbPath.string(), std::move(blobStore)));
		_forest->Create();

		_uow = _forest->CreateUnitOfWork();
		_adder = _uow->CreateFileAdderEs();
		_restorer = _uow->CreateFileRestorerEs();
		_finder = _uow->CreateFileFinder();

		// Test data
		fs::CreateDirectories(_sampleBasePath);
		fs::CreateDirectories(_sampleSubDirectory);
		CreateFile(_sampleFilePath, "hey babe");
		CreateFile(_sampleSubFilePath, "hey sub babe");
	}

	~FileRestorerEsIntegrationTest()
	{
		// Reset the UoW before the forest to ensure the transaction is rolled back
		_uow.reset();
		_forest.reset();
		boost::system::error_code ec;
		boost::filesystem::remove(_forestDbPath, ec);
		boost::filesystem::remove_all(_targetPath, ec);
		fs::RemoveAll(_restorePath, ec);
		fs::RemoveAll(_sampleBasePath, ec);
	}

	static blob::Address CreateFile(const fs::NativePath& path, const std::string& content)
	{
		const std::vector<uint8_t> binaryContent(content.begin(), content.end());
		auto f = fs::OpenFileWrite(path);
		if (!f)
		{
			throw std::runtime_error("Failed to create test file at " + path.ToString());
		}
		f.write(reinterpret_cast<const char*>(&binaryContent[0]), binaryContent.size());
		return blob::Address::CalculateFromContent(binaryContent);
	}

	const boost::filesystem::path _forestDbPath;
	const boost::filesystem::path _targetPath;
	const fs::NativePath _restorePath;
	std::unique_ptr<Forest> _forest;
	std::unique_ptr<UnitOfWork> _uow;
	std::unique_ptr<FileAdderEs> _adder;
	std::unique_ptr<FileRestorerEs> _restorer;
	std::unique_ptr<FileFinder> _finder;

	const fs::NativePath _sampleBasePath;
	const fs::NativePath _sampleSubDirectory;
	const fs::NativePath _sampleFilePath;
	const fs::NativePath _sampleSubFilePath;
};

TEST_F(FileRestorerEsIntegrationTest, Restore_ThrowsIfTargetDoNotExist)
{
	// Arrange
	const auto restorePath = fs::GenerateUniqueTempPath();
	// Act
	// Assert
	ASSERT_THROW(_restorer->Restore(std::vector<FileEvent>(), restorePath.ToString()), TargetPathNotSupportedException);
}

TEST_F(FileRestorerEsIntegrationTest, Restore_ThrowsIfTargetIsFile)
{
	// Arrange
	const auto restorePath = fs::GenerateUniqueTempPath();
	CreateFile(restorePath, "hello");
	// Act
	// Assert
	ASSERT_THROW(_restorer->Restore(std::vector<FileEvent>(), restorePath.ToString()), TargetPathNotSupportedException);
}

TEST_F(FileRestorerEsIntegrationTest, Restore_FileToFilePath)
{
	// Arrange
	const auto fileRestorePath = _restorePath / _sampleFilePath.GetFilename();
	_adder->Add(_sampleFilePath.ToString());

	std::vector<FileRestoreEvent> emittedEvents;
	_restorer->GetEventManager().Subscribe([&](const auto& fileRestoreEvent) {
		emittedEvents.push_back(fileRestoreEvent);
	});

	// Act
	const auto lastEvent = _finder->GetLastEventByPath(_sampleFilePath);
	_restorer->Restore(lastEvent, fileRestorePath.ToString());

	// Assert
	const FileRestoreEvent expectedRestoreEvent(lastEvent, fileRestorePath, FileRestoreEventAction::Restored);
	EXPECT_THAT(emittedEvents, ::testing::ElementsAre(expectedRestoreEvent));
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(fileRestorePath));
}

TEST_F(FileRestorerEsIntegrationTest, Restore_IgnoresUnsupportedEvents)
{
	// Arrange
	std::vector<FileEvent> eventsToRestore = {
		DirectoryEvent(fs::NativePath(R"(C:\hey baby)"), FileEventAction::ChangedRemoved),
		RegularFileEvent(fs::NativePath(R"(C:\hey other)"), boost::none, FileEventAction::Unsupported)
	};

	// Act
	_restorer->Restore(eventsToRestore, _restorePath.ToString());

	// Assert
	const std::vector<FileRestoreEvent> expectedEmittedEvents = {
		FileRestoreEvent(eventsToRestore[0], _restorePath / R"(C\hey baby)", FileRestoreEventAction::UnsupportedFileEvent),
		FileRestoreEvent(eventsToRestore[1], _restorePath / R"(C\hey other)", FileRestoreEventAction::UnsupportedFileEvent),
	};
	EXPECT_THAT(_restorer->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
}

TEST_F(FileRestorerEsIntegrationTest, Restore_Success)
{
	// Arrange
	_adder->Add(_sampleBasePath.ToString());

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath.ToString());

	// Assert
	const auto sampleBasePathRestored = _restorePath.AppendFullCopy(_sampleBasePath).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleBasePathRestored));
	const auto sampleSubDirectoryRestored = _restorePath.AppendFullCopy(_sampleSubDirectory).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleSubDirectoryRestored));
	const auto sampleFilePathRestored = _restorePath.AppendFullCopy(_sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));
	const auto sampleSubFilePathRestored = _restorePath.AppendFullCopy(_sampleSubFilePath);
	EXPECT_THAT(_sampleSubFilePath, HasSameFileContents(sampleSubFilePathRestored));
}

TEST_F(FileRestorerEsIntegrationTest, Restore_ToRelativeDirectorySuccess)
{
	// Arrange
	const auto restorePath = fs::GenerateShortUniqueTempPath();
	fs::CreateDirectories(restorePath);
	_adder->Add(_sampleBasePath.ToString());

	// Act
	test_utility::ScopedWorkingDirectory workingDirectory(restorePath);
	_restorer->Restore(_adder->GetEmittedEvents(), ".");

	// Assert
	const auto sampleBasePathRestored = restorePath.AppendFullCopy(_sampleBasePath).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleBasePathRestored));
	const auto sampleSubDirectoryRestored = restorePath.AppendFullCopy(_sampleSubDirectory).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleSubDirectoryRestored));
	const auto sampleFilePathRestored = restorePath.AppendFullCopy(_sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));
	const auto sampleSubFilePathRestored = restorePath.AppendFullCopy(_sampleSubFilePath);
	EXPECT_THAT(_sampleSubFilePath, HasSameFileContents(sampleSubFilePathRestored));
}

TEST_F(FileRestorerEsIntegrationTest, Restore_SkipsExistingFiles)
{
	// Arrange
	_adder->Add(_sampleBasePath.ToString());

	const auto sampleSubFilePathRestored = _restorePath.AppendFullCopy(_sampleSubFilePath);
	fs::CreateDirectories(sampleSubFilePathRestored.ParentPathCopy());
	CreateFile(sampleSubFilePathRestored, "Something else");

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath.ToString());

	// Assert
	const auto emittedEvents = _restorer->GetEmittedEvents();

	const auto sampleBasePathRestored = _restorePath.AppendFullCopy(_sampleBasePath).EnsureTrailingSlash();
	EXPECT_TRUE(fs::IsDirectory(sampleBasePathRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleBasePathRestored))));

	const auto sampleSubDirectoryRestored = _restorePath.AppendFullCopy(_sampleSubDirectory).EnsureTrailingSlash();
	EXPECT_TRUE(fs::IsDirectory(sampleSubDirectoryRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubDirectoryRestored))));

	const auto sampleFilePathRestored = _restorePath.AppendFullCopy(_sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleFilePathRestored))));

	EXPECT_THAT(_sampleSubFilePath, ::testing::Not(HasSameFileContents(sampleSubFilePathRestored)));
	EXPECT_THAT(emittedEvents, ::testing::Contains(
		::testing::AllOf(
			::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubFilePathRestored)),
			::testing::Field(&FileRestoreEvent::action, ::testing::Eq(FileRestoreEventAction::Skipped))
		)
	));
}

TEST_F(FileRestorerEsIntegrationTest, Restore_HandlesFileDirectoryClash)
{
	// Arrange
	_adder->Add(_sampleBasePath.ToString());

	// Create a file in the spot of one of the directories being restored
	const auto sampleSubDirectoryRestored = _restorePath.AppendFullCopy(_sampleSubDirectory);
	fs::CreateDirectories(sampleSubDirectoryRestored.ParentPathCopy());
	CreateFile(sampleSubDirectoryRestored, "Uhoh");

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath.ToString());

	// Assert
	const auto emittedEvents = _restorer->GetEmittedEvents();

	const auto sampleBasePathRestored = _restorePath.AppendFullCopy(_sampleBasePath).EnsureTrailingSlash();
	EXPECT_TRUE(fs::Exists(sampleBasePathRestored));

	EXPECT_TRUE(fs::IsRegularFile(sampleSubDirectoryRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(
		::testing::AllOf(
			::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubDirectoryRestored.EnsureTrailingSlashCopy())),
			::testing::Field(&FileRestoreEvent::action, ::testing::Eq(FileRestoreEventAction::FailedToCreateDirectory))
		)
	));

	const auto sampleFilePathRestored = _restorePath.AppendFullCopy(_sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));

	const auto sampleSubFilePathRestored = _restorePath.AppendFullCopy(_sampleSubFilePath);
	EXPECT_TRUE(!fs::Exists(sampleSubFilePathRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(
		::testing::AllOf(
			::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubFilePathRestored)),
			::testing::Field(&FileRestoreEvent::action, ::testing::Eq(FileRestoreEventAction::FailedToCreateDirectory))
		)
	));
}

}
}
}
}
