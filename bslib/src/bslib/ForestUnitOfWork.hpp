#pragma once

#include "bslib/Address.hpp"
#include "bslib/blob/BlobInfo.hpp"
#include "bslib/blob/BlobStore.hpp"
#include "bslib/blob/BlobInfoRepository.hpp"
#include "bslib/file/FileObjectInfo.hpp"
#include "bslib/file/FileObjectInfoRepository.hpp"
#include "bslib/sqlitepp/ScopedTransaction.hpp"
#include "bslib/UnitOfWork.hpp"

#include <boost/core/noncopyable.hpp>

#include <random>

namespace af {
namespace bslib {

class ForestUnitOfWork : public UnitOfWork
{
public:
	ForestUnitOfWork(
		sqlite3* connection,
		blob::BlobStore& blobStore,
		blob::BlobInfoRepository& blobInfoRepository,
		file::FileObjectInfoRepository& objectInfoRepository);

	void Commit() override;

	ObjectAddress CreateFileObject(const std::string& fullPath, const BlobAddress& contentBlobAddress) override;
	file::FileObjectInfo GetFileObject(const ObjectAddress& address) const override;
	BlobAddress CreateBlob(const std::vector<uint8_t>& content) override;
	std::vector<uint8_t> GetBlob(const BlobAddress& address) const override;
private:
	std::mt19937 _random;
	sqlitepp::ScopedTransaction _transaction;
	blob::BlobStore& _blobStore;
	blob::BlobInfoRepository& _blobInfoRepository;
	file::FileObjectInfoRepository& _fileObjectInfoRepository;
};


}
}