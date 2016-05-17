#include "ffs/sqlite/ScopedTransaction.hpp"

#include "ffs/sqlite/exceptions.hpp"
#include "ffs/sqlite/handles.hpp"

#include <boost/format.hpp>
#include <sqlite3.h>

namespace af {
namespace ffs {
namespace sqlite {

ScopedTransaction::ScopedTransaction(sqlite3* db)
	: _db(db)
	, _isOpen(false)
{
	sqlite::ScopedErrorMessage errorMessage;
	const auto beginResult = sqlite3_exec(_db, "BEGIN TRANSACTION", 0, 0, errorMessage);
	if (beginResult != SQLITE_OK)
	{
		throw BeginTransactionFailedException((boost::format("Failed to start transaction. SQLite error %1%: %2%") % beginResult % errorMessage).str());
	}

	_isOpen = true;
}

ScopedTransaction::~ScopedTransaction() noexcept
{
	if (_isOpen)
	{
		Rollback();
	}
}

void ScopedTransaction::Rollback()
{
	if (!_isOpen)
	{
		throw RollbackTransactionFailedException("Failed to rollback transaction. Already ended.");
	}

	sqlite::ScopedErrorMessage errorMessage;
	const auto commitResult = sqlite3_exec(_db, "ROLLBACK", 0, 0, errorMessage);
	if (commitResult != SQLITE_OK)
	{
		throw RollbackTransactionFailedException((boost::format("Failed to rollback transaction. SQLite error %1%: %2%") % commitResult % errorMessage).str());
	}

	_isOpen = false;
}

void ScopedTransaction::Commit()
{
	if (!_isOpen)
	{
		throw CommitTransactionFailedException("Failed to commit transaction. Already ended.");
	}

	sqlite::ScopedErrorMessage errorMessage;
	const auto commitResult = sqlite3_exec(_db, "COMMIT", 0, 0, errorMessage);
	if (commitResult != SQLITE_OK)
	{
		throw CommitTransactionFailedException((boost::format("Failed to commit transaction. SQLite error %1%: %2%") % commitResult % errorMessage).str());
	}

	_isOpen = false;
}

}
}
}