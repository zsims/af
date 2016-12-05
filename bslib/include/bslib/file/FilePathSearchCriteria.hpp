#pragma once

#include "bslib/file/FileEvent.hpp"
#include "bslib/Uuid.hpp"

#include <boost/optional.hpp>

#include <set>

namespace af {
namespace bslib {
namespace file {

struct FilePathSearchCriteria
{
	// optional parent path
	boost::optional<int64_t> parentPathId;
};

}
}
}