#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace af {
namespace bslib {

class InvalidAddressException : public std::runtime_error
{
public:
	explicit InvalidAddressException(const std::string& what)
		: std::runtime_error(what)
	{
	}
};

typedef std::array<uint8_t, 20> binary_address;

class Address
{
public:
	Address() { }
	Address(const void* rawBuffer, int bufferLength);
	explicit Address(const binary_address& address);
	explicit Address(const std::string& address);
	std::string ToString() const;
	binary_address ToBinary() const;
	bool operator<(const Address& rhs) const;
	bool operator==(const Address& rhs) const;
	bool operator!=(const Address& rhs) const;

	/**
	 * Calculates a new address based on the given binary content.
	 */
	static Address CalculateFromContent(const std::vector<uint8_t>& content);
private:
	binary_address _address;
};

typedef Address BlobAddress;
typedef Address ObjectAddress;

}
}
