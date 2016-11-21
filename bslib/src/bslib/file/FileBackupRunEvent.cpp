#include "bslib/file/FileBackupRunEvent.hpp"

namespace af {
namespace bslib {
namespace file {

std::string ToString(FileBackupRunEventAction action)
{
	switch (action)
	{
		case FileBackupRunEventAction::Started:
			return "Started";
		case FileBackupRunEventAction::Finished:
			return "Finished";
	};
	return "Unknown";
}

}
}
}