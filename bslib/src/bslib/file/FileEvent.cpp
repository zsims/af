#include "bslib/file/FileEvent.hpp"

namespace af {
namespace bslib {
namespace file {

std::string ToString(FileEventAction action)
{
	switch (action)
	{
		case FileEventAction::ChangedAdded:
			return "Added";
		case FileEventAction::ChangedModified:
			return "Modified";
		case FileEventAction::FailedToRead:
			return "FailedToRead";
		case FileEventAction::Unsupported:
			return "Unsupported";
		case FileEventAction::ChangedRemoved:
			return "Removed";
		case FileEventAction::Unchanged:
			return "Unchanged";
	};

	return "Unknown";
}

std::string ToString(FileType type)
{
	switch (type)
	{
		case FileType::RegularFile:
			return "RegularFile";
		case FileType::Directory:
			return "Directory";
		case FileType::Unsupported:
			return "Unsupported";
	};

	return "Unknown";
}

}
}
}