#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/EventManager.hpp"
#include "bslib/file/FileEvent.hpp"
#include "bslib/file/fs/path.hpp"

#include <boost/optional.hpp>

#include <functional>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>

namespace af {
namespace bslib {
namespace blob {
class BlobInfoRepository;
class BlobStore;
}
namespace file {
class FileEventStreamRepository;
class FilePathRepository;

/**
 * Adds files to the backup
 */
class FileAdder
{
public:
	FileAdder(
		const Uuid& backupRunId,
		std::shared_ptr<blob::BlobStore> blobStore,
		blob::BlobInfoRepository& blobInfoRepository,
		FileEventStreamRepository& fileEventStreamRepository,
		FilePathRepository& filePathRepository);

	/**
	* Adds the contents of the given file or directory to the attached backup
	* \exception PathNotFoundException File or directory doesn't exist at the given path
	* \exception SourcePathNotSupportedException The given source isn't a file or directory
	*/
	void Add(const UTF8String& sourcePath);

	const std::vector<FileEvent>& GetEmittedEvents() { return _emittedEvents; }
	EventManager<FileEvent>& GetEventManager() { return _eventManager; }
private:
	boost::optional<blob::Address> SaveFileContents(const fs::NativePath& sourcePath);

	void ScanDirectory(const fs::NativePath& sourcePath);
	void VisitPath(const fs::NativePath& sourcePath, const boost::optional<FileEvent>& previousEvent);
	void VisitFile(const fs::NativePath& sourcePath, const boost::optional<FileEvent>& previousEvent);
	void VisitDirectory(const fs::NativePath& sourcePath, const boost::optional<FileEvent>& previousEvent);
	void EmitEvent(const FileEvent& fileEvent);
	static boost::optional<FileEvent> FindPreviousEvent(
		const std::map<fs::NativePath, FileEvent>& fileEvents,
		const fs::NativePath& fullPath);

	std::unordered_map<fs::NativePath, std::unordered_map<FileType, int64_t>> _knownPaths;

	const Uuid _backupRunId;
	std::shared_ptr<blob::BlobStore> _blobStore;
	blob::BlobInfoRepository& _blobInfoRepository;
	FileEventStreamRepository& _fileEventStreamRepository;
	FilePathRepository& _filePathRepository;
	std::vector<FileEvent> _emittedEvents;
	EventManager<FileEvent> _eventManager;
};

}
}
}

