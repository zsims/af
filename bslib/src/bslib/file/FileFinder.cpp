#include "bslib/file/FileFinder.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileRefRepository.hpp"
#include "bslib/file/FileObjectRepository.hpp"

namespace af {
namespace bslib {
namespace file {

FileFinder::FileFinder(
	FileObjectRepository& fileObjectRepository,
	FileRefRepository& fileRefRepository)
	: _fileObjectRepository(fileObjectRepository)
	, _fileRefRepository(fileRefRepository)
{
}

boost::optional<FileRef> FileFinder::FindReference(const boost::filesystem::path& sourcePath) const
{
	return _fileRefRepository.FindReference(sourcePath.string());
}

boost::optional<FileObject> FileFinder::FindObjectById(foid id) const
{
	return _fileObjectRepository.FindObject(id);
}

FileObject FileFinder::GetObjectById(foid id) const
{
	return _fileObjectRepository.GetObject(id);
}

}
}
}

