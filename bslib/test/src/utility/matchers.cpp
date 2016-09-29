#include "matchers.hpp"

#include "bslib/file/fs/operations.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fstream>
#include <algorithm>

namespace af {
namespace bslib {
namespace test {
namespace utility {

bool AreContentsTheSame(const file::fs::NativePath& a, const file::fs::NativePath& b)
{
	auto streamA = file::fs::OpenFileRead(a, std::ifstream::in | std::ifstream::ate | std::ios::binary);
	auto streamB = file::fs::OpenFileRead(b, std::ifstream::in | std::ifstream::ate | std::ios::binary);

	if (streamA.fail() || streamB.fail())
	{
		return false;
	}

	// Check the size before running std::equal, otherwise it'll fall off one of the iterators if they're not the same size
	if (streamA.tellg() != streamB.tellg())
	{
		return false;
	}

	streamA.seekg(0);
	streamB.seekg(0);

	std::istreambuf_iterator<char> itA(streamA);
	std::istreambuf_iterator<char> itB(streamB);
	return std::equal(itA, std::istreambuf_iterator<char>(), itB);
}

}
}
}
}
