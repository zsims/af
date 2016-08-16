#pragma once

#include <boost/filesystem/path.hpp>

#include <ostream>

// googletest has trouble with "recursive containers" per https://github.com/google/googletest/issues/521
namespace boost {
namespace filesystem {
void PrintTo(const boost::filesystem::path& path, std::ostream* os);
}
}
