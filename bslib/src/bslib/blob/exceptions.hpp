#pragma once

#include "bslib/blob/Address.hpp"

#include <stdexcept>

namespace af {
namespace bslib {
namespace blob {

class DuplicateBlobException : public std::runtime_error
{
public:
	explicit DuplicateBlobException(const Address& address)
		: std::runtime_error("Duplicate blob with address " + address.ToString())
	{
	}
};

class BlobReadException: public std::runtime_error
{
public:
	explicit BlobReadException(const Address& address)
		: std::runtime_error("Failed to read blob with address " + address.ToString())
	{
	}
};

class AddBlobFailedException : public std::runtime_error
{
public:
	explicit AddBlobFailedException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

class BlobNotFoundException : public std::runtime_error
{
public:
	explicit BlobNotFoundException(const std::string& message)
		: std::runtime_error(message)
	{
	}

	explicit BlobNotFoundException(const Address& address)
		: std::runtime_error("Blob with the address " + address.ToString() + " could not be found")
	{
	}
};

class InvalidBlobStoreTypeException : public std::runtime_error
{
public:
	explicit InvalidBlobStoreTypeException(const std::string& message)
		: std::runtime_error(message)
	{
	}
};

}
}
}