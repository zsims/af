#include "bslib/file/FileFinder.hpp"

#include "bslib/file/exceptions.hpp"
#include "bslib/file/FileRefRepository.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"

namespace af {
namespace bslib {
namespace file {

FileFinder::FileFinder(
	FileObjectInfoRepository& fileObjectInfoRepository,
	FileRefRepository& fileRefRepository)
	: _fileObjectInfoRepository(fileObjectInfoRepository)
	, _fileRefRepository(fileRefRepository)
{
}

boost::optional<FileRef> FileFinder::FindReference(const boost::filesystem::path& sourcePath) const
{
	return _fileRefRepository.FindReference(sourcePath.string());
}

boost::optional<FileObjectInfo> FileFinder::FindObjectByAddress(const ObjectAddress& address) const
{
	return _fileObjectInfoRepository.FindObject(address);
}

FileObjectInfo FileFinder::GetObjectByAddress(const ObjectAddress& address) const
{
	return _fileObjectInfoRepository.GetObject(address);
}

}
}
}

