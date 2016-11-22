#pragma once

#include "bslib/Uuid.hpp"

#include <boost/optional.hpp>

namespace af {
namespace bslib {
namespace file {

struct FileBackupRunSearchCriteria
{
	FileBackupRunSearchCriteria(unsigned skipRuns, unsigned uniqueRunLimit)
		: skipRuns(skipRuns)
		, uniqueRunLimit(uniqueRunLimit)
	{
	}

	// optional run id to match
	boost::optional<Uuid> runId;

	// The number of (unique) runs to skip
	unsigned skipRuns;
	unsigned uniqueRunLimit;
};

}
}
}