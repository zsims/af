#pragma once

#include <stdexcept>
#include <string>

namespace af {
namespace bslib {

/**
 * A transaction couldn't be started.
 */
class BeginTransactionFailedException : public std::runtime_error
{
public:
	explicit BeginTransactionFailedException(const std::string& message)
		: std::runtime_error("Failed to begin transaction")
	{
	}
};

/**
 * A transaction couldn't be committed
 */
class CommitTransactionFailedException : public std::runtime_error
{
public:
	explicit CommitTransactionFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

/**
 * A transaction couldn't be rolled back
 */
class RollbackTransactionFailedException : public std::runtime_error
{
public:
	explicit RollbackTransactionFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

/**
 * A statement could not be prepared
 */
class PrepareStatementFailedException : public std::runtime_error
{
public:
	explicit PrepareStatementFailedException(int sqliteError)
		: std::runtime_error("Failed to prepare statement, error " + std::to_string(sqliteError))
	{
	}
};

class ExecuteFailedException : public std::runtime_error
{
public:
	explicit ExecuteFailedException(int sqliteError)
		: std::runtime_error("Failed to execute statement, error " + std::to_string(sqliteError))
	{
	}
};

class OpenDatabaseFailedException : public std::runtime_error
{
public:
	OpenDatabaseFailedException(const std::string& path, int sqliteError)
		: std::runtime_error("Cannot open database at " + path + ". SQLite returned " + std::to_string(sqliteError))
	{
	}
};

class BindParameterFailedException : public std::runtime_error
{
public:
	BindParameterFailedException(const std::string& name, int sqliteError)
		: std::runtime_error("Failed to bind paramter " + name + ". SQLite returned " + std::to_string(sqliteError))
	{
	}
};

}
}
