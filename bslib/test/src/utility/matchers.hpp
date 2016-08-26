#pragma once

#include <boost/filesystem/path.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <fstream>
#include <algorithm>

namespace blib {
namespace test {
namespace utility {

bool AreContentsTheSame(const boost::filesystem::path& a, const boost::filesystem::path& b)
{
	std::ifstream streamA(a.string(), std::ifstream::in | std::ifstream::ate | std::ios::binary);
	std::ifstream streamB(b.string(), std::ifstream::in | std::ifstream::ate | std::ios::binary);

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

MATCHER_P(HasSameFileContents, value, "") { return blib::test::utility::AreContentsTheSame(arg, value); }
