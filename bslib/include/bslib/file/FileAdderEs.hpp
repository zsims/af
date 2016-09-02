#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileEvent.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <vector>
#include <map>

namespace af {
namespace bslib {
namespace blob {
class BlobInfoRepository;
class BlobStore;
}
namespace file {
class FileEventStreamRepository;

/**
 * Adds files to the backup
 */
class FileAdderEs
{
public:
	FileAdderEs(
		blob::BlobStore& blobStore,
		blob::BlobInfoRepository& blobInfoRepository,
		FileEventStreamRepository& fileEventStreamRepository);

	/**
	* Adds the contents of the given file or directory to the attached backup
	* \exception PathNotFoundException File or directory doesn't exist at the given path
	* \exception SourcePathNotSupportedException The given source isn't a file or directory
	*/
	void Add(const boost::filesystem::path& sourcePath);
private:
	boost::optional<BlobAddress> SaveFileContents(const boost::filesystem::path& sourcePath);

	void ScanDirectory(const boost::filesystem::path& sourcePath);
	void VisitPath(const boost::filesystem::path& sourcePath);
	void VisitFile(const boost::filesystem::path& sourcePath, const boost::optional<FileEvent>& previousEvent);
	void VisitDirectory(const boost::filesystem::path& sourcePath);

	blob::BlobStore& _blobStore;
	blob::BlobInfoRepository& _blobInfoRepository;
	FileEventStreamRepository& _fileEventStreamRepository;

	boost::optional<FileEvent> FindPreviousEvent(const boost::filesystem::path& fullPath) const;
	void PushEvent(const FileEvent& fileEvent);

	// For building state
	std::map<boost::filesystem::path, FileEvent> _lastEvents;
	std::vector<FileEvent> _newEvents;
};

}
}
}

