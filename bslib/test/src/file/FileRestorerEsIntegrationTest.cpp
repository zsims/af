#include "bslib/forest.hpp"
#include "bslib/blob/DirectoryBlobStore.hpp"
#include "bslib/file/path_util.hpp"
#include "bslib/file/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"
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
		, _restorePath(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path())
		, _sampleBasePath(EnsureTrailingSlash(boost::filesystem::temp_directory_path() / boost::filesystem::unique_path()))
		, _sampleSubDirectory(_sampleBasePath / boost::filesystem::unique_path())
		, _sampleFilePath(_sampleBasePath / "base.dat")
		, _sampleSubFilePath(_sampleSubDirectory / "subfile.dat")
	{
		boost::filesystem::create_directories(_targetPath);
		boost::filesystem::create_directories(_restorePath);
		auto blobStore = std::make_unique<blob::DirectoryBlobStore>(_targetPath);
		_forest.reset(new Forest(_forestDbPath.string(), std::move(blobStore)));
		_forest->Create();

		_uow = _forest->CreateUnitOfWork();
		_adder = _uow->CreateFileAdderEs();
		_restorer = _uow->CreateFileRestorerEs();
		_finder = _uow->CreateFileFinder();

		// Test data
		boost::filesystem::create_directories(_sampleBasePath);
		boost::filesystem::create_directories(_sampleSubDirectory);
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
		boost::filesystem::remove_all(_restorePath, ec);
		boost::filesystem::remove_all(_sampleBasePath, ec);
	}

	static blob::Address CreateFile(const boost::filesystem::path& path, const std::string& content)
	{
		const std::vector<uint8_t> binaryContent(content.begin(), content.end());
		std::ofstream f(path.string(), std::ofstream::out | std::ofstream::binary);
		if (!f)
		{
			throw std::runtime_error("Failed to create test file at " + path.string());
		}
		f.write(reinterpret_cast<const char*>(&binaryContent[0]), binaryContent.size());
		return blob::Address::CalculateFromContent(binaryContent);
	}

	const boost::filesystem::path _forestDbPath;
	const boost::filesystem::path _targetPath;
	const boost::filesystem::path _restorePath;
	std::unique_ptr<Forest> _forest;
	std::unique_ptr<UnitOfWork> _uow;
	std::unique_ptr<FileAdderEs> _adder;
	std::unique_ptr<FileRestorerEs> _restorer;
	std::unique_ptr<FileFinder> _finder;

	const boost::filesystem::path _sampleBasePath;
	const boost::filesystem::path _sampleSubDirectory;
	const boost::filesystem::path _sampleFilePath;
	const boost::filesystem::path _sampleSubFilePath;
};

TEST_F(FileRestorerEsIntegrationTest, Restore_ThrowsIfTargetDoNotExist)
{
	// Arrange
	const auto restorePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	// Act
	// Assert
	ASSERT_THROW(_restorer->Restore(std::vector<FileEvent>(), restorePath), TargetPathNotSupportedException);
}

TEST_F(FileRestorerEsIntegrationTest, Restore_ThrowsIfTargetIsFile)
{
	// Arrange
	const auto restorePath = boost::filesystem::temp_directory_path() / boost::filesystem::unique_path();
	CreateFile(restorePath, "hello");
	// Act
	// Assert
	ASSERT_THROW(_restorer->Restore(std::vector<FileEvent>(), restorePath), TargetPathNotSupportedException);
}

TEST_F(FileRestorerEsIntegrationTest, Restore_FileToFilePath)
{
	// Arrange
	const auto fileRestorePath = _restorePath / _sampleFilePath.filename();
	_adder->Add(_sampleFilePath);

	std::vector<FileRestoreEvent> emittedEvents;
	_restorer->GetEventManager().Subscribe([&](const auto& fileRestoreEvent) {
		emittedEvents.push_back(fileRestoreEvent);
	});

	// Act
	const auto lastEvent = _finder->GetLastEventByPath(_sampleFilePath);
	_restorer->Restore(lastEvent, fileRestorePath);

	// Assert
	const FileRestoreEvent expectedRestoreEvent(lastEvent, fileRestorePath, FileRestoreEventAction::Restored);
	EXPECT_THAT(emittedEvents, ::testing::ElementsAre(expectedRestoreEvent));
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(fileRestorePath));
}

TEST_F(FileRestorerEsIntegrationTest, Restore_IgnoresUnsupportedEvents)
{
	// Arrange
	std::vector<FileEvent> eventsToRestore = {
		DirectoryEvent("/hey baby", FileEventAction::ChangedRemoved),
		RegularFileEvent("/hey other", boost::none, FileEventAction::Unsupported)
	};

	// Act
	_restorer->Restore(eventsToRestore, _restorePath);

	// Assert
	const std::vector<FileRestoreEvent> expectedEmittedEvents = {
		FileRestoreEvent(eventsToRestore[0], _restorePath / "hey baby", FileRestoreEventAction::UnsupportedFileEvent),
		FileRestoreEvent(eventsToRestore[1], _restorePath / "hey other", FileRestoreEventAction::UnsupportedFileEvent),
	};
	EXPECT_THAT(_restorer->GetEmittedEvents(), ::testing::UnorderedElementsAreArray(expectedEmittedEvents));
}

TEST_F(FileRestorerEsIntegrationTest, Restore_Success)
{
	// Arrange
	_adder->Add(_sampleBasePath);

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath);

	// Assert
	const auto sampleBasePathRestored = EnsureTrailingSlash(CombineFullPaths(_restorePath, _sampleBasePath));
	EXPECT_TRUE(boost::filesystem::exists(sampleBasePathRestored));
	const auto sampleSubDirectoryRestored = EnsureTrailingSlash(CombineFullPaths(_restorePath, _sampleSubDirectory));
	EXPECT_TRUE(boost::filesystem::exists(sampleSubDirectoryRestored));
	const auto sampleFilePathRestored = CombineFullPaths(_restorePath, _sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));
	const auto sampleSubFilePathRestored = CombineFullPaths(_restorePath, _sampleSubFilePath);
	EXPECT_THAT(_sampleSubFilePath, HasSameFileContents(sampleSubFilePathRestored));
}

TEST_F(FileRestorerEsIntegrationTest, Restore_SkipsExistingFiles)
{
	// Arrange
	_adder->Add(_sampleBasePath);

	const auto sampleSubFilePathRestored = CombineFullPaths(_restorePath, _sampleSubFilePath);
	boost::filesystem::create_directories(sampleSubFilePathRestored.parent_path());
	CreateFile(sampleSubFilePathRestored, "Something else");

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath);

	// Assert
	const auto emittedEvents = _restorer->GetEmittedEvents();

	const auto sampleBasePathRestored = EnsureTrailingSlash(CombineFullPaths(_restorePath, _sampleBasePath));
	EXPECT_TRUE(boost::filesystem::is_directory(sampleBasePathRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleBasePathRestored))));

	const auto sampleSubDirectoryRestored = EnsureTrailingSlash(CombineFullPaths(_restorePath, _sampleSubDirectory));
	EXPECT_TRUE(boost::filesystem::is_directory(sampleSubDirectoryRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(sampleSubDirectoryRestored))));

	const auto sampleFilePathRestored = CombineFullPaths(_restorePath, _sampleFilePath);
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
	_adder->Add(_sampleBasePath);

	// Create a file in the spot of one of the directories being restored
	const auto sampleSubDirectoryRestored = CombineFullPaths(_restorePath, _sampleSubDirectory);
	boost::filesystem::create_directories(sampleSubDirectoryRestored.parent_path());
	CreateFile(sampleSubDirectoryRestored, "Uhoh");

	// Act
	_restorer->Restore(_adder->GetEmittedEvents(), _restorePath);

	// Assert
	const auto emittedEvents = _restorer->GetEmittedEvents();

	const auto sampleBasePathRestored = EnsureTrailingSlash(CombineFullPaths(_restorePath, _sampleBasePath));
	EXPECT_TRUE(boost::filesystem::exists(sampleBasePathRestored));

	EXPECT_TRUE(boost::filesystem::is_regular_file(sampleSubDirectoryRestored));
	EXPECT_THAT(emittedEvents, ::testing::Contains(
		::testing::AllOf(
			::testing::Field(&FileRestoreEvent::targetPath, ::testing::Eq(EnsureTrailingSlashCopy(sampleSubDirectoryRestored))),
			::testing::Field(&FileRestoreEvent::action, ::testing::Eq(FileRestoreEventAction::FailedToCreateDirectory))
		)
	));

	const auto sampleFilePathRestored = CombineFullPaths(_restorePath, _sampleFilePath);
	EXPECT_THAT(_sampleFilePath, HasSameFileContents(sampleFilePathRestored));

	const auto sampleSubFilePathRestored = CombineFullPaths(_restorePath, _sampleSubFilePath);
	EXPECT_TRUE(!boost::filesystem::exists(sampleSubFilePathRestored));
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
