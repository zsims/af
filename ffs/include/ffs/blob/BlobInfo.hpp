#pragma once

#include "ffs/Address.hpp"

#include <memory>

namespace af {
namespace ffs {
namespace blob {

class BlobInfo
{
public:
	BlobInfo(const BlobAddress& address, uint64_t sizeBytes)
		: _address(address)
		, _sizeBytes(sizeBytes)
	{
	}

	const BlobAddress GetAddress() const { return _address; }
	const uint64_t GetSizeBytes() const { return _sizeBytes; }

	bool operator==(const BlobInfo& rhs) const { return _address == rhs.GetAddress() && _sizeBytes == rhs.GetSizeBytes(); }

private:
	const BlobAddress _address;
	const uint64_t _sizeBytes;
};

}
}
}