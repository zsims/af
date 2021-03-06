#include "bslib/file/fs/WindowsPath.hpp"

#include "bslib/file/exceptions.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>

#include <algorithm>

namespace af {
namespace bslib {
namespace file {
namespace fs {

namespace {
const char SEPARATOR = u'\\';
// Prefix for extended paths per https://msdn.microsoft.com/en-au/library/windows/desktop/aa365247(v=vs.85).aspx#short_vs._long_names
const UTF8String EXTENDED_PATH_PREFIX(R"(\\?\)");

bool StartsWithSeparator(const UTF8String& p)
{
	return p.length() >= 1 && p[0] == SEPARATOR;
}

bool EndsWithSeparator(const UTF8String& p)
{
	const auto length = p.length();
	return length >= 1 && p[length - 1] == SEPARATOR;
}

}

WindowsPath::WindowsPath()
	: _path(EXTENDED_PATH_PREFIX)
{
}

WindowsPath::WindowsPath(const UTF8String& path)
	: _path(path)
{
	EnsureExtendedPath();
}

WindowsPath::WindowsPath(const char* path)
	: WindowsPath(UTF8String(path))
{
}

WindowsPath& WindowsPath::EnsureTrailingSlash()
{
	if (!EndsWithSeparator(_path))
	{
		_path.push_back(SEPARATOR);
	}
	return *this;
}

WindowsPath WindowsPath::EnsureTrailingSlashCopy() const
{
	if (!EndsWithSeparator(_path))
	{
		return WindowsPath(_path + SEPARATOR);
	}
	return WindowsPath(*this);
}

void WindowsPath::EnsureExtendedPath()
{
	if (!boost::starts_with(_path, EXTENDED_PATH_PREFIX))
	{
		_path.insert(0, EXTENDED_PATH_PREFIX);
	}
}

void WindowsPath::AppendSegment(const UTF8String& segment)
{
	// boost::filesystem::path does this, such that appending an empty segment does nothing
	if (segment.empty())
	{
		return;
	}

	if (!EndsWithSeparator(_path) && !StartsWithSeparator(segment))
	{
		_path.push_back(SEPARATOR);
	}
	_path.append(segment);
}

void WindowsPath::AppendFull(const WindowsPath& p)
{
	auto sanitized = p.ToNormalString();
	// TODO add other special separators, plus handle streams, e.g. C:\Foo\bar:xx.txt
	boost::replace_all(sanitized, ":", "");
	AppendSegment(sanitized);
}

WindowsPath WindowsPath::AppendFullCopy(const WindowsPath& p) const
{
	auto copy = *this;
	copy.AppendFull(p);
	return copy;
}

UTF8String WindowsPath::GetFilename() const
{
	if (_path.empty())
	{
		return UTF8String();
	}
	auto realEnd = _path.end() - 1;
	if (EndsWithSeparator(_path))
	{
		realEnd--;
	}
	for (auto it = realEnd; it != _path.begin(); it--)
	{
		if (*it == SEPARATOR)
		{
			return UTF8String(it + 1, realEnd + 1);
		}
	}
	return UTF8String();
}

WindowsPath WindowsPath::ParentPathCopy() const
{
	const auto index = _path.find_last_of(SEPARATOR);
	if (index != UTF8String::npos)
	{
		return WindowsPath(UTF8String(_path.begin(), _path.begin() + index));
	}
	return WindowsPath();
}

std::vector<WindowsPath> WindowsPath::GetIntermediatePaths() const
{
	std::vector<WindowsPath> result;
	// Can assume this starts with a separator
	auto i = EXTENDED_PATH_PREFIX.length();
	const auto end = _path.length() - 1;
	for(;;)
	{
		i = _path.find_first_of(SEPARATOR, i);
		// Done
		if (i == UTF8String::npos || i == end)
		{
			result.push_back(WindowsPath(UTF8String(_path.begin(), _path.end())));
			break;
		}
		++i;
		result.push_back(WindowsPath(UTF8String(_path.begin(), _path.begin() + i)));
	}

	return result;
}

UTF8String WindowsPath::ToNormalString() const
{
	if (boost::starts_with(_path, EXTENDED_PATH_PREFIX))
	{
		return UTF8String(_path.begin() + EXTENDED_PATH_PREFIX.length(), _path.end());
	}
	return _path;
}

void WindowsPath::MakePreferred()
{
	boost::replace_all(_path, "/", R"(\)");
}

unsigned WindowsPath::GetDepth() const
{
	auto separatorCount = std::count(_path.begin() + EXTENDED_PATH_PREFIX.length(), _path.end(), '\\');
	// Don't count trailing separators for depth
	if (_path.length() > EXTENDED_PATH_PREFIX.length() && *(_path.end() - 1) == '\\')
	{
		separatorCount--;
	}

	return static_cast<unsigned>(separatorCount);
}

}
}
}
}