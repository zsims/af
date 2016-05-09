#pragma once

#include <array>
#include <cstdint>

namespace af {
namespace ffs {

// E.g. a SHA-1
typedef std::array<uint32_t, 5> Address;

typedef Address BlobAddress;

}
}