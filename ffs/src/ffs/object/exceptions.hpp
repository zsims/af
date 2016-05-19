#pragma once

#include <stdexcept>

namespace af {
namespace ffs {
namespace object {

class DuplicateObjectException : public std::runtime_error
{
public:
	explicit DuplicateObjectException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class ObjectNotFoundException : public std::runtime_error
{
public:
	explicit ObjectNotFoundException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class AddObjectFailedException : public std::runtime_error
{
public:
	explicit AddObjectFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class GetObjectsFailedException : public std::runtime_error
{
public:
	explicit GetObjectsFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

}
}
}