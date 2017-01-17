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
	VirtualFile result(match.pathId, match.fullPath, match.pathType);
	result.matchedFileEvent = match.latestEvent;
	return result;
}

}

VirtualFileBrowser::VirtualFileBrowser(FileEventStreamRepository& fileEventStreamRepository, const boost::optional<boost::posix_time::ptime>& atUtc)
	: _fileEventStreamRepository(fileEventStreamRepository)
	, _atUtc(atUtc)
{
}

std::vector<VirtualFile> VirtualFileBrowser::ListRoots(unsigned skip, unsigned limit) const
{
	FilePathSearchCriteria pathCriteria;
	pathCriteria.rootPath = true;
	return List(pathCriteria, skip, limit);
}

std::vector<VirtualFile> VirtualFileBrowser::ListContents(int64_t pathId, unsigned skip, unsigned limit) const
{
	FilePathSearchCriteria pathCriteria;
	pathCriteria.parentPathId = pathId;
	return List(pathCriteria, skip, limit);
}

std::unordered_map<int64_t, unsigned> VirtualFileBrowser::CountNestedMatches(const std::unordered_set<int64_t>& pathIds) const
{
	FileEventSearchCriteria eventCriteria;
	eventCriteria.actions = MATCH_EVENTS;
	eventCriteria.before = _atUtc;
	return _fileEventStreamRepository.CountNestedMatches(eventCriteria, pathIds);
}

std::vector<VirtualFile> VirtualFileBrowser::List(const FilePathSearchCriteria& pathCriteria, unsigned skip, unsigned limit) const
{
	FileEventSearchCriteria eventCriteria;
	eventCriteria.actions = MATCH_EVENTS;
	eventCriteria.before = _atUtc;
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

