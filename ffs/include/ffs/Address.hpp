#pragma once

#include <array>
#include <cstdint>

namespace af {
namespace ffs {

class InvalidAddressException : public std::runtime_error
{
public:
	explicit InvalidAddressException(const std::string& what)
		: std::runtime_error(what)
	{
	}
};

// E.g. a SHA-1
typedef std::array<uint8_t, 20> binary_address;

class Address
{
public:
	explicit Address(const binary_address& address);
	explicit Address(const std::string& address);
	std::string ToString() const;
	binary_address ToBinary() const;
	bool operator<(const Address& rhs) const;
	bool operator==(const Address& rhs) const;
private:
	binary_address _address;
};

typedef Address BlobAddress;
typedef Address ObjectAddress;

}
}
