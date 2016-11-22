#pragma once

#include "bslib/Uuid.hpp"

#include <boost/optional.hpp>

namespace af {
namespace bslib {
namespace file {

struct FileBackupRunSearchCriteria
{
	// optional run id to match
	boost::optional<Uuid> runId;
};

}
}
}