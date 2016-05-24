#pragma once

#include "ffs/Address.hpp"
#include "ffs/blob/BlobInfo.hpp"
#include "ffs/blob/BlobStore.hpp"
#include "ffs/blob/BlobInfoRepository.hpp"
#include "ffs/object/ObjectInfo.hpp"
#include "ffs/object/ObjectInfoRepository.hpp"
#include "ffs/sqlitepp/ScopedTransaction.hpp"
#include "ffs/UnitOfWork.hpp"

#include <boost/core/noncopyable.hpp>

#include <random>

namespace af {
namespace ffs {

class ForestUnitOfWork : public UnitOfWork
{
public:
	ForestUnitOfWork(
		sqlite3* connection,
		std::shared_ptr<blob::BlobStore> blobStore,
		std::shared_ptr<blob::BlobInfoRepository> blobInfoRepository,
		std::shared_ptr<object::ObjectInfoRepository> objectInfoRepository);

	void Commit() override;

	ObjectAddress CreateObject(const std::string& type, const object::ObjectBlobList& objectBlobs) override;
	object::ObjectInfo GetObject(const ObjectAddress& address) const override;
	BlobAddress CreateBlob(const std::vector<uint8_t>& content) override;
	std::vector<uint8_t> GetBlob(const BlobAddress& address) const override;
private:
	std::mt19937 _random;
	sqlitepp::ScopedTransaction _transaction;
	std::shared_ptr<blob::BlobStore> _blobStore;
	std::shared_ptr<blob::BlobInfoRepository> _blobInfoRepository;
	std::shared_ptr<object::ObjectInfoRepository> _objectInfoRepository;
};


}
}