#pragma once

#include <stdexcept>

namespace af {
namespace ffs {

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

}
}
