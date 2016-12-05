#include "bslib/file/VirtualFileBrowser.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"

#include <vector>

namespace af {
namespace bslib {
namespace file {

namespace {
const std::set<FileEventAction> MATCH_EVENTS{
	FileEventAction::ChangedAdded,
	FileEventAction::ChangedModified,
	FileEventAction::ChangedRemoved,
	FileEventAction::Unchanged
};
const std::set<FileEventAction> REDUCE_EVENTS{
	FileEventAction::ChangedAdded,
	FileEventAction::ChangedModified,
	FileEventAction::Unchanged
};

VirtualFile ToVirtualFile(const FileEvent& fileEvent)
{
	return VirtualFile(fileEvent.fullPath, fileEvent.type);
}

}

VirtualFileBrowser::VirtualFileBrowser(FileEventStreamRepository& fileEventStreamRepository)
	: _fileEventStreamRepository(fileEventStreamRepository)
{
}

std::vector<VirtualFile> VirtualFileBrowser::List(unsigned skip, unsigned limit) const
{
	//FileEventSearchCriteria criteria;
	//criteria.actions = MATCH_EVENTS;
	//const auto events = _fileEventStreamRepository.SearchDistinctPath(criteria, REDUCE_EVENTS, skip, limit);
	//std::vector<VirtualFile> result;
	//for (const auto& ev : events)
	//{
	//	result.push_back(ToVirtualFile(ev));
	//}
	//return result;
	std::vector<VirtualFile> result;
	return result;
}

}
}
}

