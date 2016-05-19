#pragma once

struct sqlite3;

namespace af {
namespace ffs {
namespace sqlitepp {

class ScopedTransaction
{
public:
	ScopedTransaction(const ScopedTransaction& that) = delete;
	ScopedTransaction& operator=(const ScopedTransaction&) = delete;

	/**
	 * Start a new scoped transaction.
	 * \throws BeginTransactionFailedException The transaction cannot be committed.
	 */
	explicit ScopedTransaction(sqlite3* db);
	virtual ~ScopedTransaction() noexcept;

	/**
	 * Rolls back the transaction, this is the default if it's not commited
	 * \throws RollbackTransactionFailedException The transaction cannot be committed.
	 */
	void Rollback();

	/**
	 * Commit the transaction.
	 * \throws CommitTransactionFailedException The transaction cannot be committed.
	 */
	void Commit();
protected:
	bool _isOpen;
	sqlite3* _db;
};


}
}
}