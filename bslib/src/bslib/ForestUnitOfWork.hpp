#pragma once

#include "bslib/Address.hpp"
#include "bslib/blob/BlobInfo.hpp"
#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/FileEvent.hpp"
#include "bslib/file/FileEventStreamRepository.hpp"
#include "bslib/file/FileRefRepository.hpp"
#include "bslib/file/FileObject.hpp"
#include "bslib/file/FileObjectRepository.hpp"
#include "bslib/sqlitepp/ScopedTransaction.hpp"
#include "bslib/UnitOfWork.hpp"

#include <boost/core/noncopyable.hpp>

namespace af {
namespace bslib {

class ForestUnitOfWork : public UnitOfWork
{
public:
	ForestUnitOfWork(const sqlitepp::ScopedSqlite3Object& connection, blob::BlobStore& blobStore);

	void Commit() override;

	std::unique_ptr<file::FileAdder> CreateFileAdder() override;
	std::unique_ptr<file::FileAdderEs> CreateFileAdderEs() override;
	std::unique_ptr<file::FileRestorerEs> CreateFileRestorerEs() override;
	std::unique_ptr<file::FileFinder> CreateFileFinder() override;
	std::vector<uint8_t> GetBlob(const BlobAddress& address) const override;
private:
	sqlitepp::ScopedTransaction _transaction;
	blob::BlobStore& _blobStore;
	blob::BlobInfoRepository _blobInfoRepository;
	file::FileObjectRepository _fileObjectRepository;
	file::FileRefRepository _fileRefRepository;
	file::FileEventStreamRepository _fileEventStreamRepository;
};


}
}