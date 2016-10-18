#pragma once

#include "bslib/file/fs/path.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace af {
namespace bslib_test_util {

bool AreContentsTheSame(const bslib::file::fs::NativePath& a, const bslib::file::fs::NativePath& b);

}
}

MATCHER_P(HasSameFileContents, value, "") { return af::bslib_test_util::AreContentsTheSame(arg, value); }
