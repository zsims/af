#pragma once

#include "ffs/sqlitepp/ScopedTransaction.hpp"

struct sqlite3;

namespace af {
namespace ffs {

/**
 * Represents a "unit of work" allowing multiple repositories to be called atomically.
 * If you don't have one of these babies in scope, then that's on you.
 */
class ScopedUnitOfWork
{
public:
	ScopedUnitOfWork(const ScopedUnitOfWork& that) = delete;
	ScopedUnitOfWork& operator=(const ScopedUnitOfWork&) = delete;

	explicit ScopedUnitOfWork(sqlite3* connection)
		: _transaction(connection)
	{
	}
	virtual ~ScopedUnitOfWork() noexcept { }
	void Commit() { _transaction.Commit(); }
private:
	sqlitepp::ScopedTransaction _transaction;
};


}
}