#pragma once

#include "bslib/blob/Address.hpp"

#include <memory>

namespace af {
namespace bslib {
namespace blob {

class BlobInfo
{
public:
	BlobInfo(const Address& address, uint64_t sizeBytes)
		: _address(address)
		, _sizeBytes(sizeBytes)
	{
	}

	const Address GetAddress() const { return _address; }
	const uint64_t GetSizeBytes() const { return _sizeBytes; }

	bool operator==(const BlobInfo& rhs) const { return _address == rhs.GetAddress() && _sizeBytes == rhs.GetSizeBytes(); }

private:
	const Address _address;
	const uint64_t _sizeBytes;
};

}
}
}