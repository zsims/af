#pragma once

#include "bslib/file/VirtualFile.hpp"

#include <vector>

namespace af {
namespace bslib {
namespace file {
class FileEventStreamRepository;

/**
 * Provides a view of files from what is stored in the backup
 */
class VirtualFileBrowser
{
public:
	explicit VirtualFileBrowser(FileEventStreamRepository& fileEventStreamRepository);

	/**
	 * Lists all virtual files
	 */
	std::vector<VirtualFile> List(unsigned skip, unsigned limit) const;
private:
	FileEventStreamRepository& _fileEventStreamRepository;
};

}
}
}

