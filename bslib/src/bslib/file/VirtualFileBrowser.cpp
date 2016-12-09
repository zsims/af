#include "bslib/file/VirtualFileBrowser.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"

#include <vector>

namespace af {
namespace bslib {
namespace file {

namespace {
const std::set<FileEventAction> MATCH_EVENTS {
	FileEventAction::ChangedAdded,
	FileEventAction::ChangedModified,
	FileEventAction::ChangedRemoved,
	FileEventAction::Unchanged
};

VirtualFile ToVirtualFile(const FileEventStreamRepository::PathFirstSearchMatch& match)
{
	auto pathType = FileType::Directory;
	if (match.latestEvent)
	{
		pathType = match.latestEvent->type;
	}
	VirtualFile result(match.fullPath, pathType);
	result.matchedFileEvent = match.latestEvent;
	return result;
}

}

VirtualFileBrowser::VirtualFileBrowser(FileEventStreamRepository& fileEventStreamRepository)
	: _fileEventStreamRepository(fileEventStreamRepository)
{
}

std::vector<VirtualFile> VirtualFileBrowser::ListRoots(unsigned skip, unsigned limit) const
{
	FilePathSearchCriteria pathCriteria;
	pathCriteria.rootPath = true;
	FileEventSearchCriteria eventCriteria;
	eventCriteria.actions = MATCH_EVENTS;
	const auto matches = _fileEventStreamRepository.SearchPathFirst(pathCriteria, eventCriteria, skip, limit);
	std::vector<VirtualFile> result;
	for (const auto& match : matches)
	{
		result.push_back(ToVirtualFile(match));
	}
	return result;
}

}
}
}

