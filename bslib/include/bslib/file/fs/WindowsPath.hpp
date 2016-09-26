#pragma once

#include "bslib/unicode.hpp"

#include <string>
#include <vector>

namespace af {
namespace bslib {
namespace file {
namespace fs {

/**
 * Models a full Windows path.
 * Unlike boost::filesystem::path this supports:
 *  - Wide characters
 *  - Extended/long paths (e.g. \\?\C:\foo)
 *  - Is not POSIX centric
 *  - Is cross platform (e.g. the semantics don't change based on the current platform)
 * This class provides the following loose guarantees:
 *  - Long path support
 *  - Paths will always use \ separators on Windows, / characters are not treated as separators due to implications with extended paths
 * Assumes:
 *  - You always provide a full path, although this isn't a strict requirement -- relative paths are not well tested ;)
 */
class WindowsPath
{
public:
	WindowsPath();

	/**
	 * Constructs a path from a UTF-8 string
	 */
	explicit WindowsPath(const UTF8String& path);
	explicit WindowsPath(const char* path);

	/**
	 * Constructs a path from a wide string
	 */
	explicit WindowsPath(const std::wstring& path);
	explicit WindowsPath(const wchar_t* path);

	/**
	 * Ensures that the given path has a trailing separator
	 */
	WindowsPath& EnsureTrailingSlash();
	WindowsPath EnsureTrailingSlashCopy() const;

	/**
	 * Returns the filename portion of the path
	 */
	UTF8String GetFilename() const;

	/**
	 * Returns the parent of the path, e.g. given C:\Foo\bar this is C:\Foo
	 */
	WindowsPath ParentPathCopy() const;

	/**
	 * Returns the intermediate paths that make up the given path, including the path itself.
	 * For example WindowsPath("C:\foo\bar.txt").GetIntermediatePaths() = ["\\?\C:\", "\\?\C:\foo\", "\\?\C:\foo\bar.txt"]
	 */
	std::vector<WindowsPath> GetIntermediatePaths() const;

	/**
	 * Returns the "extended" path (with the extended prefix)
	 */
	const UTF8String& ToExtendedString() const { return _path; }
	std::wstring ToExtendedWideString() const { return UTF8ToWideString(_path); }

	/**
	 * Returns the "normal" path (without the extended prefix)
	 */
	UTF8String ToNormalString() const;
	std::wstring ToNormalWideString() const { return UTF8ToWideString(ToNormalString()); }

	/**
	 * Appends the given segment
	 */
	void AppendSegment(const UTF8String& segment);

	/**
	 * Appends the given full path, e.g. "C:\foo\bar".Append(WindowsPath("D:\yeah\baby")) == "C:\foo\bar\D\yeah\baby"
	 */
	void AppendFull(const WindowsPath& p);

	bool operator==(const WindowsPath& rhs) const
	{
		return _path == rhs._path;
	}
	
	bool operator!=(const WindowsPath& rhs) const
	{
		return _path != rhs._path;
	}

	bool operator==(const UTF8String& rhs) const
	{
		return _path == rhs;
	}

	bool operator<(const WindowsPath& rhs) const
	{
		return _path < rhs._path;
	}

	WindowsPath& operator/=(const UTF8String& p)
	{
		AppendSegment(p);
		return *this;
	}
private:
	void EnsureExtendedPath();
	UTF8String _path;
};

inline WindowsPath operator/(const WindowsPath& lhs, const UTF8String& rhs) { return WindowsPath(lhs) /= rhs; }
inline WindowsPath operator/(const WindowsPath& lhs, const char* rhs) { return WindowsPath(lhs) /= UTF8String(rhs); }

}
}
}
}