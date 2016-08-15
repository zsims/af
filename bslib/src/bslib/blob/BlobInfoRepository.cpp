
#include "bslib/blob/BlobInfoRepository.hpp"

#include "bslib/blob/BlobInfo.hpp"
#include "bslib/blob/exceptions.hpp"
#include "bslib/sqlitepp/sqlitepp.hpp"

#include <boost/format.hpp>
#include <sqlite3.h>

#include <memory>

namespace af {
namespace bslib {
namespace blob {

namespace {
enum GetAllBlobsColumnIndex
{
	GetAllBlobs_ColumnIndex_Address = 0,
	GetAllBlobs_ColumnIndex_SizeBytes
};

enum FindBlobColumnIndex
{
	FindBlob_ColumnIndex_SizeBytes = 0
};
}

BlobInfoRepository::BlobInfoRepository(const sqlitepp::ScopedSqlite3Object& connection)
	: _db(connection)
{
	sqlitepp::prepare_or_throw(_db, "INSERT INTO Blob (Address, SizeBytes) VALUES (:Address, :SizeBytes)", _insertBlobStatement);
	sqlitepp::prepare_or_throw(_db, "SELECT Address, SizeBytes FROM Blob", _getAllBlobsStatement);
	sqlitepp::prepare_or_throw(_db, "SELECT SizeBytes FROM Blob WHERE Address = :Address", _findBlobStatement);
}

std::vector<std::shared_ptr<BlobInfo>> BlobInfoRepository::GetAllBlobs() const
{
	std::vector<std::shared_ptr<BlobInfo>> result;
	sqlitepp::ScopedStatementReset reset(_getAllBlobsStatement);

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(_getAllBlobsStatement)) == SQLITE_ROW)
	{
		const auto addressBytesCount = sqlite3_column_bytes(_getAllBlobsStatement, GetAllBlobs_ColumnIndex_Address);
		const auto addressBytes = sqlite3_column_blob(_getAllBlobsStatement, GetAllBlobs_ColumnIndex_Address);
		BlobAddress address(addressBytes, addressBytesCount);
		const auto sizeBytes = static_cast<uint64_t>(sqlite3_column_int64(_getAllBlobsStatement, GetAllBlobs_ColumnIndex_SizeBytes));
		result.push_back(std::make_shared<BlobInfo>(address, sizeBytes));
	}
	return result;
}

void BlobInfoRepository::AddBlob(const BlobInfo& info)
{
	// binary address, note this has to be kept in scope until SQLite has finished as we've opted not to make a copy
	const auto binaryAddress = info.GetAddress().ToBinary();
	sqlitepp::ScopedStatementReset reset(_insertBlobStatement);
	sqlitepp::BindByParameterNameBlob(_insertBlobStatement, ":Address", &binaryAddress[0], binaryAddress.size());
	sqlitepp::BindByParameterNameInt64(_insertBlobStatement, ":SizeBytes", static_cast<int64_t>(info.GetSizeBytes()));

	// execute
	const auto stepResult = sqlite3_step(_insertBlobStatement);
	if (stepResult != SQLITE_DONE)
	{
		if (stepResult == SQLITE_CONSTRAINT)
		{
			throw DuplicateBlobException(info.GetAddress());
		}
		throw AddBlobFailedException((boost::format("Failed to execute statement for insert blob %1%. SQLite error %2%") % info.GetAddress().ToString() % stepResult).str());
	}
}

std::unique_ptr<BlobInfo> BlobInfoRepository::FindBlob(const BlobAddress& address)
{
	const auto binaryAddress = address.ToBinary();
	sqlitepp::ScopedStatementReset reset(_findBlobStatement);
	sqlitepp::BindByParameterNameBlob(_findBlobStatement, ":Address", &binaryAddress[0], binaryAddress.size());

	const auto stepResult = sqlite3_step(_findBlobStatement);
	if (stepResult != SQLITE_ROW)
	{
		return std::unique_ptr<BlobInfo>();
	}

	const auto sizeBytes = static_cast<uint64_t>(sqlite3_column_int64(_findBlobStatement, FindBlob_ColumnIndex_SizeBytes));
	return std::make_unique<BlobInfo>(address, sizeBytes);
}

}
}
}