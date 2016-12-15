#pragma once

#include "bslib/file/VirtualFile.hpp"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace af {
namespace bslib {
namespace file {
class FileEventStreamRepository;
struct FilePathSearchCriteria;

/**
 * Provides a view of files from what is stored in the backup
 */
class VirtualFileBrowser
{
public:
	explicit VirtualFileBrowser(FileEventStreamRepository& fileEventStreamRepository);
	std::vector<VirtualFile> ListRoots(unsigned skip, unsigned limit) const;
	std::vector<VirtualFile> ListContents(int64_t pathId, unsigned skip, unsigned limit) const;
	std::unordered_map<int64_t, unsigned> CountNestedMatches(const std::unordered_set<int64_t>& pathIds) const;
private:
	std::vector<VirtualFile> List(const FilePathSearchCriteria& pathCriteria, unsigned skip, unsigned limit) const;
	FileEventStreamRepository& _fileEventStreamRepository;
};

}
}
}

