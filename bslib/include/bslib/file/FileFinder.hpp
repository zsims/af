#pragma once

#include "bslib/Address.hpp"
#include "bslib/file/FileRef.hpp"
#include "bslib/file/FileObjectInfo.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <vector>

namespace af {
namespace bslib {
namespace file {

class FileRefRepository;
class FileObjectInfoRepository;

/**
 * Finds files in a backup
 */
class FileFinder
{
public:
	FileFinder(FileObjectInfoRepository& fileObjectInfoRepository, FileRefRepository& fileRefRepository);

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
	boost::optional<FileObjectInfo> FindObjectByAddress(const ObjectAddress& address) const;

	/**
	 * Gets an object by address
	 * \param address The object address
	 * \return The object info
	 * \throws FileObjectNotFoundException if no object exists at that address
	 */
	FileObjectInfo GetObjectByAddress(const ObjectAddress& address) const;
private:
	FileObjectInfoRepository& _fileObjectInfoRepository;
	FileRefRepository& _fileRefRepository;
};

}
}
}

