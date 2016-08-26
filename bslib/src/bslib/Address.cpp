#include "bslib/Address.hpp"

#include <boost/uuid/sha1.hpp>

#include <sstream>
#include <iomanip>
#include <algorithm>

namespace af {
namespace bslib {

Address::Address(const void* rawBuffer, int bufferLength)
{
	if (bufferLength != static_cast<int>(_address.max_size()))
	{
		throw InvalidAddressException("Given buffer is not of the correct size, expected " + _address.max_size());
	}
	std::copy_n(static_cast<const uint8_t*>(rawBuffer), bufferLength, _address.begin());
}

Address::Address(const binary_address& address)
	: _address(address)
{
}

Address::Address(const std::string& address)
{
	const auto length = 40;
	if (address.length() != length)
	{
		throw InvalidAddressException("Given address is not a valid string-encoded address");
	}
	
	auto ai = 0;
	for (auto i = 0; i < length; i += 2)
	{
		// The stream overload for hex will treat the parsing differently if the target is a char
		int tmp;
		std::istringstream(address.substr(i, 2)) >> std::hex >> tmp;
		_address[ai] = static_cast<uint8_t>(tmp);
		ai++;
	}
}

bool Address::operator<(const Address& rhs) const
{
	return _address < rhs._address;
}

bool Address::operator==(const Address& rhs) const
{
	return _address == rhs._address;
}

bool Address::operator!=(const Address& rhs) const
{
	return _address != rhs._address;
}

binary_address Address::ToBinary() const
{
	return _address;
}

std::string Address::ToString() const
{
	std::ostringstream result;
	for (auto c: _address)
	{
		result << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint16_t>(c);
	}
	return result.str();
}

Address Address::CalculateFromContent(const std::vector<uint8_t>& content)
{
	if (content.size() == 0)
	{
		// Null value hash for SHA1
		return Address("da39a3ee5e6b4b0d3255bfef95601890afd80709");
	}

	boost::uuids::detail::sha1 sha;
	sha.process_bytes(&content[0], content.size());
	unsigned int digest[5];
	sha.get_digest(digest);
	auto i = 0;
	binary_address hash;
	for (auto d : digest)
	{
		hash[i++] = (d >> 24) & 0xFF;
		hash[i++] = (d >> 16) & 0xFF;
		hash[i++] = (d >> 8) & 0xFF;
		hash[i++] = d & 0xFF;
	}
	return Address(hash);
}

}
}

