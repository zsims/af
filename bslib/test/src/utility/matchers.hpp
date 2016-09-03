#pragma once

#include <boost/filesystem/path.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace blib {
namespace test {
namespace utility {

bool AreContentsTheSame(const boost::filesystem::path& a, const boost::filesystem::path& b);

}
}
}

MATCHER_P(HasSameFileContents, value, "") { return blib::test::utility::AreContentsTheSame(arg, value); }
