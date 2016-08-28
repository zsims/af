#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileRef.hpp"
#include "bslib/file/FileObject.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

class FileRefRepository;
class FileObjectRepository;

/**
 * Finds files in a backup
 */
class FileFinder
{
public:
	FileFinder(FileObjectRepository& fileObjectRepository, FileRefRepository& fileRefRepository);

	/**
	 * Finds a reference by source path.
	 * \param sourcePath the full source path
	 * \return The file reference if found, otherwise none
	 */
	boost::optional<FileRef> FindReference(const boost::filesystem::path& sourcePath) const;

	/**
	 * Finds an object by address
	 * \param address The object address
	 * \return The object info if found, otherwise none
	 */
	boost::optional<FileObject> FindObjectById(foid address) const;

	/**
	 * Gets an object by address
	 * \param address The object address
	 * \return The object info
	 * \throws FileObjectNotFoundException if no object exists at that address
	 */
	FileObject GetObjectById(foid id) const;
private:
	FileObjectRepository& _fileObjectRepository;
	FileRefRepository& _fileRefRepository;
};

}
}
}

