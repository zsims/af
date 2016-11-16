#pragma once

#include "bslib/blob/Address.hpp"
#include "bslib/file/FileBackupRunRecorder.hpp"
#include "bslib/file/FileAdder.hpp"
#include "bslib/file/FileFinder.hpp"
#include "bslib/file/FileRestorer.hpp"
#include "bslib/Uuid.hpp"

#include <boost/core/noncopyable.hpp>

#include <memory>
#include <string>

namespace af {
namespace bslib {

class UnitOfWork : private boost::noncopyable
{
public:
	UnitOfWork() { }
	virtual ~UnitOfWork() { }

	/**
	 * Saves the new unit of work
	 */
	virtual void Commit() = 0;

	/**
	 * Creates a file backup run recorder for recording details of a backup run
	 */
	virtual std::unique_ptr<file::FileBackupRunRecorder> CreateFileBackupRunRecorder() = 0;

	/**
	 * Creates a file adder for backing up files and directories.
	 */
	virtual std::unique_ptr<file::FileAdder> CreateFileAdder(const Uuid& backupRunId) = 0;

	/**
	 * Creates a file restorer for restoring files and directories.
	 */
	virtual std::unique_ptr<file::FileRestorer> CreateFileRestorer() = 0;

	/**
	 * Creates a file finder for finding files/directories in the backup
	 */
	virtual std::unique_ptr<file::FileFinder> CreateFileFinder() = 0;

	/**
	 * Gets a blob by address.
	 * \exception BlobReadException The blob with the given address couldn't be read, e.g. it doesn't exist or a permissions failure.
	 */
	virtual std::vector<uint8_t> GetBlob(const blob::Address& address) const = 0;
};


}
}
