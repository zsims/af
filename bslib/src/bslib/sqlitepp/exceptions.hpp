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

class RegisterFunctionFailedException : public std::runtime_error
{
public:
	explicit RegisterFunctionFailedException(int sqliteError)
		: std::runtime_error("Failed to register function, error " + std::to_string(sqliteError))
	{
	}
};

/**
 * A statement could not be prepared
 */
class PrepareStatementFailedException : public std::runtime_error
{
public:
	PrepareStatementFailedException(int sqliteError, const std::string& description)
		: std::runtime_error("Failed to prepare statement, error " + std::to_string(sqliteError) + " " + description)
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
		: std::runtime_error("Failed to bind parameter " + name + ". SQLite returned " + std::to_string(sqliteError))
	{
	}
};

class InitializeBackupFailedException : public std::runtime_error
{
public:
	explicit InitializeBackupFailedException(int sqliteError)
		: std::runtime_error("Failed initialize backup as part of SQLite online backup. SQLite returned " + std::to_string(sqliteError))
	{
	}
};

class BackupStepFailedException : public std::runtime_error
{
public:
	explicit BackupStepFailedException(int sqliteError)
		: std::runtime_error("Failed transfer data between two databases as part of SQLite online backup. SQLite returned " + std::to_string(sqliteError))
	{
	}
};

class EmptySetLiteralException : public std::runtime_error
{
public:
	explicit EmptySetLiteralException()
		: std::runtime_error("Produced set literal is empty")
	{
	}
};

}
}
