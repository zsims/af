#include "bslib/file/FileBackupRunReader.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileBackupRunEventStreamRepository.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace af {
namespace bslib {
namespace file {

FileBackupRunReader::FileBackupRunReader(FileBackupRunEventStreamRepository& backupRunEventRepository)
	: _backupRunEventRepository(backupRunEventRepository)
{
}

}
}
}

