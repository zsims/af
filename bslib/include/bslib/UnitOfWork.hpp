#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileAdderEs.hpp"
#include "bslib/file/FileFinder.hpp"
#include "bslib/file/FileRestorerEs.hpp"

#include <boost/core/noncopyable.hpp>

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
	 * Creates a file adder for backing up files and directories.
	 */
	virtual std::unique_ptr<file::FileAdderEs> CreateFileAdderEs() = 0;

	/**
	 * Creates a file restorer for restoring files and directories.
	 */
	virtual std::unique_ptr<file::FileRestorerEs> CreateFileRestorerEs() = 0;

	/**
	 * Creates a file finder for finding files/directories in the backup
	 */
	virtual std::unique_ptr<file::FileFinder> CreateFileFinder() = 0;

	/**
	 * Gets a blob by address.
	 * \exception BlobReadException The blob with the given address couldn't be read, e.g. it doesn't exist or a permissions failure.
	 */
	virtual std::vector<uint8_t> GetBlob(const BlobAddress& address) const = 0;
};


}
}
