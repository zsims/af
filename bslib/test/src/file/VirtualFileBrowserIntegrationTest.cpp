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
		: _backupRunId(Uuid::Create())
		, _eventCUsersZsimsVimRc(_backupRunId, fs::NativePath(R"(C:\Users\zsims\_vimrc)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded)
		, _eventCUsersZsimsTax2017Ato(_backupRunId, fs::NativePath(R"(C:\Users\zsims\Tax\2017.ato)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded)
		, _eventCUsersZsimsPictures(_backupRunId, fs::NativePath(R"(C:\Users\zsims\Pictures\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded)
		, _eventCUsersPicturesSamsonJpg(_backupRunId, fs::NativePath(R"(C:\Users\zsims\Pictures\Samson.jpg)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded)
		, _eventCWtfTempFile(_backupRunId, fs::NativePath(R"(C:\wtf\temp)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded)
		, _eventRemovedCWtfTempFile(_backupRunId, fs::NativePath(R"(C:\wtf\temp)"), FileType::RegularFile, boost::none, FileEventAction::ChangedRemoved)
		, _eventCWtfTempFolder(_backupRunId, fs::NativePath(R"(C:\wtf\temp)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded)
		, _eventD(_backupRunId, fs::NativePath(R"(D:\)"), FileType::Directory, boost::none, FileEventAction::Unchanged)
		, _eventDMovies(_backupRunId, fs::NativePath(R"(D:\Movies\)"), FileType::Directory, boost::none, FileEventAction::Unchanged)
		, _eventDMoviesPersonal(_backupRunId, fs::NativePath(R"(D:\Movies\Personal\)"), FileType::Directory, boost::none, FileEventAction::ChangedAdded)
		, _eventDMoviesPersonalSamsonsFirstStepsAvi(_backupRunId, fs::NativePath(R"(D:\Movies\Personal\Samson's first steps.avi)"), FileType::RegularFile, boost::none, FileEventAction::ChangedAdded)
	{
		_testBackup.OpenOrCreate();
		_connection = _testBackup.ConnectToDatabase();
		_fileEventStreamRepository = std::make_unique<FileEventStreamRepository>(*_connection);
		_filePathRepository = std::make_unique<FilePathRepository>(*_connection);

		// Sample data
		_filePathRepository->AddPathTree(fs::NativePath(R"(C:\Users\zsims\Downloads\Temp\)"), FileType::Directory);
		AddEvent(_eventCUsersZsimsVimRc);
		AddEvent(_eventCUsersZsimsTax2017Ato);
		AddEvent(_eventCUsersZsimsPictures);
		AddEvent(_eventCUsersPicturesSamsonJpg);
		AddEvent(_eventCWtfTempFile);
		AddEvent(_eventRemovedCWtfTempFile);
		AddEvent(_eventCWtfTempFolder);
		AddEvent(_eventD);
		AddEvent(_eventDMovies);
		AddEvent(_eventDMoviesPersonal);
		AddEvent(_eventDMoviesPersonalSamsonsFirstStepsAvi);
	}
	
	void AddEvent(const FileEvent& fileEvent)
	{
		const auto pathId = _filePathRepository->AddPathTree(fileEvent.fullPath, fileEvent.type);
		_fileEventStreamRepository->AddEvent(fileEvent, pathId);
	}

	static boost::optional<VirtualFile> FindByPath(const fs::NativePath& path, FileType type, const std::vector<VirtualFile>& files)
	{
		const auto it = std::find_if(files.begin(), files.end(), [&](const VirtualFile& file) {
			return file.fullPath == path && file.type == type;
		});

		if (it != files.end())
		{
			return *it;
		}

		return boost::none;
	}

	static unsigned CountNestedMatches(const VirtualFileBrowser& browser, int64_t pathId)
	{
		return browser.CountNestedMatches({ pathId }).at(pathId);
	}

	const Uuid _backupRunId;
	const FileEvent _eventCUsersZsimsVimRc;
	const FileEvent _eventCUsersZsimsTax2017Ato;
	const FileEvent _eventCUsersZsimsPictures;
	const FileEvent _eventCUsersPicturesSamsonJpg;
	const FileEvent _eventCWtfTempFile;
	const FileEvent _eventRemovedCWtfTempFile;
	const FileEvent _eventCWtfTempFolder;
	const FileEvent _eventD;
	const FileEvent _eventDMovies;
	const FileEvent _eventDMoviesPersonal;
	const FileEvent _eventDMoviesPersonalSamsonsFirstStepsAvi;

	std::unique_ptr<sqlitepp::ScopedSqlite3Object> _connection;
	std::unique_ptr<FileEventStreamRepository> _fileEventStreamRepository;
	std::unique_ptr<FilePathRepository> _filePathRepository;

};

TEST_F(VirtualFileBrowserIntegrationTest, ListRoots_Success)
{
	// Arrange
	const auto uow = _testBackup.GetBackup().CreateUnitOfWork();
	const auto browser = uow->CreateVirtualFileBrowser();
	// Act
	const auto roots = browser->ListRoots(0, 100);
	// Assert
	ASSERT_EQ(2, roots.size());
	{
		const auto root = FindByPath(fs::NativePath(R"(C:\)"), FileType::Directory, roots);
		ASSERT_TRUE(root);
		// Note that C:\wtf\temp is both a file and a folder
		EXPECT_EQ(6, CountNestedMatches(*browser, root->pathId));
		EXPECT_FALSE(root->matchedFileEvent);
	}
	{
		const auto root = FindByPath(fs::NativePath(R"(D:\)"), FileType::Directory, roots);
		ASSERT_TRUE(root);
		EXPECT_EQ(3, CountNestedMatches(*browser, root->pathId));
		ASSERT_TRUE(root->matchedFileEvent);
		EXPECT_EQ(_eventD, root->matchedFileEvent.value());
	}
}

TEST_F(VirtualFileBrowserIntegrationTest, ListContents_Success)
{
	// Arrange
	const auto uow = _testBackup.GetBackup().CreateUnitOfWork();
	const auto browser = uow->CreateVirtualFileBrowser();
	const auto pathId = _filePathRepository->FindPath(fs::NativePath(R"(C:\Users\zsims\)"), FileType::Directory);
	ASSERT_TRUE(pathId);
	// Act
	const auto contents = browser->ListContents(pathId.value(), 0, 100);

	// Assert
	ASSERT_EQ(4, contents.size());
	{
		const auto item = FindByPath(fs::NativePath(R"(C:\Users\zsims\_vimrc)"), FileType::RegularFile, contents);
		ASSERT_TRUE(item);
		EXPECT_EQ(FileType::RegularFile, item->type);
		EXPECT_EQ(0, CountNestedMatches(*browser, item->pathId));
		ASSERT_TRUE(item->matchedFileEvent);
		EXPECT_EQ(_eventCUsersZsimsVimRc, item->matchedFileEvent.value());
	}
	{
		const auto item = FindByPath(fs::NativePath(R"(C:\Users\zsims\Pictures\)"), FileType::Directory, contents);
		ASSERT_TRUE(item);
		EXPECT_EQ(FileType::Directory, item->type);
		EXPECT_EQ(1, CountNestedMatches(*browser, item->pathId));
		ASSERT_TRUE(item->matchedFileEvent);
		EXPECT_EQ(_eventCUsersZsimsPictures, item->matchedFileEvent.value());
	}
	{
		const auto item = FindByPath(fs::NativePath(R"(C:\Users\zsims\Tax\)"), FileType::Directory, contents);
		ASSERT_TRUE(item);
		EXPECT_EQ(FileType::Directory, item->type);
		EXPECT_EQ(1, CountNestedMatches(*browser, item->pathId));
		EXPECT_FALSE(item->matchedFileEvent);
	}
	{
		const auto item = FindByPath(fs::NativePath(R"(C:\Users\zsims\Downloads\)"), FileType::Directory, contents);
		ASSERT_TRUE(item);
		EXPECT_EQ(FileType::Directory, item->type);
		EXPECT_EQ(0, CountNestedMatches(*browser, item->pathId));
		EXPECT_FALSE(item->matchedFileEvent);
	}

	// These two paths are the same, but C:\temp existed as both a file and a folder
}

TEST_F(VirtualFileBrowserIntegrationTest, ListContents_ChangedTypesSuccess)
{
	// Arrange
	const auto uow = _testBackup.GetBackup().CreateUnitOfWork();
	const auto browser = uow->CreateVirtualFileBrowser();
	const auto pathId = _filePathRepository->FindPath(fs::NativePath(R"(C:\wtf\)"), FileType::Directory);
	ASSERT_TRUE(pathId);
	// Act
	const auto contents = browser->ListContents(pathId.value(), 0, 100);
	// Assert
	ASSERT_EQ(2, contents.size());
	{
		const auto item = FindByPath(fs::NativePath(R"(C:\wtf\temp)"), FileType::RegularFile, contents);
		ASSERT_TRUE(item);
		EXPECT_EQ(FileType::RegularFile, item->type);
		EXPECT_EQ(0, CountNestedMatches(*browser, item->pathId));
		EXPECT_TRUE(item->matchedFileEvent);
	}
	{
		const auto item = FindByPath(fs::NativePath(R"(C:\wtf\temp)"), FileType::Directory, contents);
		ASSERT_TRUE(item);
		EXPECT_EQ(FileType::Directory, item->type);
		EXPECT_EQ(0, CountNestedMatches(*browser, item->pathId));
		ASSERT_TRUE(item->matchedFileEvent);
		EXPECT_EQ(_eventCWtfTempFolder, item->matchedFileEvent.value());
	}
}


TEST_F(VirtualFileBrowserIntegrationTest, CountNestedMatches_RootSuccess)
{
	// Arrange
	const auto cPathId = _filePathRepository->FindPath(fs::NativePath(R"(C:\)"), FileType::Directory);
	const auto dPathId = _filePathRepository->FindPath(fs::NativePath(R"(D:\)"), FileType::Directory);
	ASSERT_TRUE(cPathId);
	ASSERT_TRUE(dPathId);
	const auto uow = _testBackup.GetBackup().CreateUnitOfWork();
	const auto browser = uow->CreateVirtualFileBrowser();

	// Act
	const auto counts = browser->CountNestedMatches({ cPathId.value(), dPathId.value() });

	// Assert
	EXPECT_EQ(6, counts.at(cPathId.value()));
	EXPECT_EQ(3, counts.at(dPathId.value()));
}

TEST_F(VirtualFileBrowserIntegrationTest, CountNestedMatches_ContentsSuccess)
{
	// Arrange
	const auto cUsersZsimsPathId = _filePathRepository->FindPath(fs::NativePath(R"(C:\Users\zsims\)"), FileType::Directory);
	const auto cUsersZsimsTaxPathId = _filePathRepository->FindPath(fs::NativePath(R"(C:\Users\zsims\Tax\)"), FileType::Directory);
	const auto cUsersZsimsPicturesPathId = _filePathRepository->FindPath(fs::NativePath(R"(C:\Users\zsims\Pictures\)"), FileType::Directory);
	ASSERT_TRUE(cUsersZsimsPathId);
	ASSERT_TRUE(cUsersZsimsTaxPathId);
	ASSERT_TRUE(cUsersZsimsPicturesPathId);
	const auto uow = _testBackup.GetBackup().CreateUnitOfWork();
	const auto browser = uow->CreateVirtualFileBrowser();

	// Act
	// Assert
	{
		const auto counts = browser->CountNestedMatches({ cUsersZsimsPathId.value() });
		EXPECT_EQ(4, counts.at(cUsersZsimsPathId.value()));
	}
	{
		const auto counts = browser->CountNestedMatches({ cUsersZsimsTaxPathId.value() });
		EXPECT_EQ(1, counts.at(cUsersZsimsTaxPathId.value()));
	}
	{
		const auto counts = browser->CountNestedMatches({ cUsersZsimsPicturesPathId.value() });
		EXPECT_EQ(1, counts.at(cUsersZsimsPicturesPathId.value()));
	}
}

}
}
}
}
