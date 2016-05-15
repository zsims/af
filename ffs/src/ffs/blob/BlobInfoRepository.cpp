
#include "ffs/blob/BlobInfoRepository.hpp"

#include "ffs/blob/BlobInfo.hpp"
#include "ffs/blob/exceptions.hpp"
#include "ffs/exceptions.hpp"

#include <boost/format.hpp>
#include <sqlite3.h>

#include <algorithm>
#include <memory>

namespace af {
namespace ffs {
namespace blob {

BlobInfoRepository::BlobInfoRepository(const std::string& utf8DbPath)
	: _db(0)
{
	const auto result = sqlite3_open_v2(utf8DbPath.c_str(), &_db, SQLITE_OPEN_READWRITE, 0);
	if (result != SQLITE_OK)
	{
		throw OpenDatabaseFailedException((boost::format("Cannot open database at %1%. SQLite returned %2%") % utf8DbPath % result).str());
	}
}

BlobInfoRepository::~BlobInfoRepository()
{
	sqlite3_close_v2(_db);
}

std::vector<BlobInfoModelPtr> BlobInfoRepository::GetAllBlobs() const
{
	std::vector<BlobInfoModelPtr> result;

	{
		char* errorMessage = 0;
		const auto beginResult = sqlite3_exec(_db, "BEGIN TRANSACTION", 0, 0, &errorMessage);
		if (beginResult != SQLITE_OK)
		{
			//sqlite3_free(errorMessage);
			throw GetBlobsFailedException((boost::format("Failed to start transaction. SQLite error %1%: %2%") % beginResult % errorMessage).str());
		}
	}

	sqlite3_stmt* statement = 0;
	const auto prepareResult = sqlite3_prepare_v2(_db, "SELECT Address, SizeBytes FROM Blob", -1, &statement, 0);
	if (prepareResult != SQLITE_OK)
	{
		throw GetBlobsFailedException((boost::format("Failed to prepare get all blobs statement. SQLite error %1%") % prepareResult).str());
	}

	auto stepResult = 0;
	while ((stepResult = sqlite3_step(statement)) == SQLITE_ROW)
	{
		binary_address address;
		const auto addressBytesCount = sqlite3_column_bytes(statement, 0);
		const auto addressBytes = sqlite3_column_blob(statement, 0);

		if (addressBytesCount > static_cast<int>(address.max_size()))
		{
			throw GetBlobsFailedException((boost::format("Failed to read blob address, too big (%1% bytes).") % addressBytes).str());
		}

		std::copy_n(static_cast<const uint8_t*>(addressBytes), addressBytesCount, address.begin());
		const auto sizeBytes = static_cast<uint64_t>(sqlite3_column_int64(statement, 1));
		result.push_back(std::make_shared<BlobInfo>(BlobAddress(address), sizeBytes));
	}

	sqlite3_finalize(statement);

	{
		char* errorMessage = 0;
		const auto rollbackResult = sqlite3_exec(_db, "ROLLBACK", 0, 0, &errorMessage);
		if (rollbackResult != SQLITE_OK)
		{
			//sqlite3_free(errorMessage);
			throw GetBlobsFailedException((boost::format("Failed to rollback transaction for read blobs. SQLite error %1%: %2%") % rollbackResult % errorMessage).str());
		}
	}

	return result;
}

void BlobInfoRepository::AddBlob(const BlobInfo& info)
{
	// binary address, note this has to be kept in scope until SQLite has finished as we've opted not to make a copy
	const auto binaryAddress = info.GetAddress().ToBinary();
	{
		char* errorMessage = 0;
		const auto beginResult = sqlite3_exec(_db, "BEGIN TRANSACTION", 0, 0, &errorMessage);
		if (beginResult != SQLITE_OK)
		{
			//sqlite3_free(errorMessage);
			throw AddBlobFailedException((boost::format("Failed to start transaction. SQLite error %1%: %2%") % beginResult % errorMessage).str());
		}
	}

	sqlite3_stmt* statement = 0;
	const auto prepareResult = sqlite3_prepare_v2(_db, "INSERT INTO Blob (Address, SizeBytes) VALUES (?, ?)", -1, &statement, 0);
	if (prepareResult != SQLITE_OK)
	{
		// Note that statement doesn't need to be free'd if prepare fails
		throw AddBlobFailedException((boost::format("Failed to prepare blob insert statement for %1%. SQLite error %2%") % info.GetAddress().ToString() % prepareResult).str());
	}

	{
		const auto bindResult = sqlite3_bind_blob(statement, 1, &binaryAddress[0], static_cast<int>(binaryAddress.size()), 0);
		if (bindResult != SQLITE_OK)
		{
			throw AddBlobFailedException((boost::format("Failed to bind blob parameter address %1%. SQLite error %2%") % info.GetAddress().ToString() % bindResult).str());
		}
	}

	{
		const auto bindResult = sqlite3_bind_int64(statement, 2, static_cast<int64_t>(info.GetSizeBytes()));
		if (bindResult != SQLITE_OK)
		{
			throw AddBlobFailedException((boost::format("Failed to bind blob parameter size %1%. SQLite error %2%") % info.GetAddress().ToString() % bindResult).str());
		}
	}

	// execute
	const auto stepResult = sqlite3_step(statement);
	if (stepResult != SQLITE_DONE)
	{
		if (stepResult == SQLITE_CONSTRAINT)
		{
			throw DuplicateBlobException((boost::format("Attempted to insert duplicate blob %1%. SQLite error %2%") % info.GetAddress().ToString() % stepResult).str());
		}
		throw AddBlobFailedException((boost::format("Failed to execute statement for insert blob. SQLite error %2%") % info.GetAddress().ToString() % stepResult).str());
	}

	sqlite3_finalize(statement);

	{
		char* errorMessage = 0;
		const auto commitResult = sqlite3_exec(_db, "COMMIT", 0, 0, &errorMessage);
		if (commitResult != SQLITE_OK)
		{
			//sqlite3_free(errorMessage);
			throw AddBlobFailedException((boost::format("Failed to commit insert blob for blob %1%. SQLite error %2%: %3%") % info.GetAddress().ToString() % commitResult % errorMessage).str());
		}
	}
}

}
}
}