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
	const UTF8String& ToString() const { return _path; }
	const UTF8String& ToExtendedString() const { return _path; }

	/**
	 * Returns the "normal" path (without the extended prefix)
	 */
	UTF8String ToNormalString() const;

	/**
	 * Appends the given segment
	 */
	void AppendSegment(const UTF8String& segment);

	/**
	 * Appends the given full path, e.g. "C:\foo\bar".Append(WindowsPath("D:\yeah\baby")) == "C:\foo\bar\D\yeah\baby"
	 */
	void AppendFull(const WindowsPath& p);
	WindowsPath AppendFullCopy(const WindowsPath& p) const;

	/**
	 * Converts forward slashes to back slashes
	 */
	void MakePreferred();

	/**
	 * Gets the depth of the path based on how many components it has.
	 * \\C:\ has a depth of 0
	 * \\C:\foo has a depth of 1
	 * \\C:\foo\bar\ has a depth of 2
	 */
	unsigned GetDepth() const;

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



	/**
	 * Checks if the given rhs path is a sub path of the lhs, and optionally checks if it has the given depth beyond lhs
	 * \remarks This is a static function to permit usage from C-like scenarios without constructing a full path
	 * \returns True if the given paths are equal (a path is its own child), else true if the rhs is a child path of lhs, else true if the rhs is a child path of lhs with the specified depth
	 */
	static bool IsChildPath(const char* lhsUtf8, const char* rhsUtf8, int maxDepth = -1);
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

namespace std {
template <>
struct hash<af::bslib::file::fs::WindowsPath>
{
	std::size_t operator()(const af::bslib::file::fs::WindowsPath& k) const
	{
		return std::hash<std::string>()(k.ToString());
	}
};

}