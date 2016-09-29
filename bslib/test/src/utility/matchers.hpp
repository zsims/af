#pragma once

#include "bslib/file/fs/path.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace af {
namespace bslib {
namespace test {
namespace utility {

bool AreContentsTheSame(const file::fs::NativePath& a, const file::fs::NativePath& b);

}
}
}
}

MATCHER_P(HasSameFileContents, value, "") { return af::bslib::test::utility::AreContentsTheSame(arg, value); }
