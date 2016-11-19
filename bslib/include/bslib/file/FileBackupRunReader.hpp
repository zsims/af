#pragma once

#include "bslib/file/FileBackupRunEvent.hpp"
#include "bslib/Uuid.hpp"

namespace af {
namespace bslib {
namespace file {
class FileBackupRunEventStreamRepository;

class FileBackupRunReader
{
public:
	explicit FileBackupRunReader(FileBackupRunEventStreamRepository& backupRunEventRepository);
private:
	FileBackupRunEventStreamRepository& _backupRunEventRepository;
};

}
}
}

