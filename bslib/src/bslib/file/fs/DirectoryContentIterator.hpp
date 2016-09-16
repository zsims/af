#pragma once

#include "bslib/file/fs/WindowsPath.hpp"

#include <boost/assert.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <windows.h>

#include <memory>

namespace af {
namespace bslib {
namespace file {
namespace fs {

/**
 * Represents an entry within a directory
 */
struct DirectoryEntry
{
	DirectoryEntry(const WindowsPath& fullPath)
		: fullPath(fullPath)
	{
	}
	WindowsPath fullPath;
};

/**
 * Iterates content in the given directory
 * Results are returned such that:
 *  - A full path is built for each entry
 *  - Directories will have trailing slashes, e.g. C:\Foo\Bar\
 *  - Symbolic links are returned, but not resolved
 *  - . and .. elements are included
 */
class DirectoryContentIterator : public boost::iterator_facade<DirectoryContentIterator, const DirectoryEntry&, boost::forward_traversal_tag>
{
public:
	DirectoryContentIterator()
		: _handle(nullptr)
	{
	}

	explicit DirectoryContentIterator(const WindowsPath& p, boost::system::error_code& ec) noexcept
		: _path(p)
		, _handle(nullptr)
	{
		/*
			TODO: If you are writing a 32-bit application to list all the files in a directory and the application may be run on a 64-bit computer,
			you should call Wow64DisableWow64FsRedirection before calling FindFirstFileEx and call Wow64RevertWow64FsRedirection after the last call
			to FindNextFile. For more information, see File System Redirector.
			- https://msdn.microsoft.com/en-us/library/windows/desktop/aa364419(v=vs.85).aspx
		*/
		WIN32_FIND_DATAW findData = { 0 };

		// Build a search path, e.g. C:\foo\bar\*
		const auto searchPath = p / "*";

		// Handle root directory searches, per 
		// "Note  Prepending the string "\\?\" does not allow access to the root directory." - https://msdn.microsoft.com/en-us/library/windows/desktop/aa364419(v=vs.85).aspx
		// To work around this, don't use \\?\ for paths shorter than MAX_PATH
		auto searchString = searchPath.ToNormalWideString();
		if (searchString.size() >= MAX_PATH)
		{
			searchString = searchPath.ToExtendedWideString();
		}

		auto handle = FindFirstFileExW(
			reinterpret_cast<const wchar_t*>(searchString.c_str()),
			FindExInfoBasic,
			&findData,
			FindExSearchNameMatch,
			nullptr,
			0);

		if (handle != INVALID_HANDLE_VALUE)
		{
			_handle.reset(handle, ::FindClose);
			BuildEntry(findData);
			ec = boost::system::error_code();
		}
		else
		{
			const auto lastError = ::GetLastError();
			// An empty root path (e.g. C:\) has no . or .. elements, so treat ERROR_FILE_NOT_FOUND as a non error
			ec = boost::system::error_code(lastError == ERROR_NO_MORE_FILES ? 0 : lastError, boost::system::system_category());
		}
	}

	void increment(boost::system::error_code& ec) noexcept
	{
		BOOST_ASSERT(_handle);
		WIN32_FIND_DATAW data = { 0 };
		if (FindNextFileW(_handle.get(), &data) == FALSE)
		{
			const auto lastError = ::GetLastError();
			if (lastError != ERROR_NO_MORE_FILES)
			{
				ec = boost::system::error_code(lastError, boost::system::system_category());
			}
			else
			{
				ec = boost::system::error_code();
			}

			// signal end of the iterator
			_handle.reset();
		}
		else
		{
			ec = boost::system::error_code();
			BuildEntry(data);
		}
	}
private:
	friend class boost::iterator_core_access;

	void BuildEntry(const WIN32_FIND_DATAW& data) noexcept
	{
		static_assert(sizeof(wchar_t) == sizeof(char16_t), "Expected sizeof wchar_t to be the same as sizeof char16_t");

		// Build a full path
		auto fullPath = _path;
		const auto utf8Filename = WideToUTF8String(data.cFileName);
		fullPath.AppendSegment(utf8Filename);

		// ensure directories have trailing slashes, except for . and .. references
		if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
			wcscmp(data.cFileName, L".") != 0 &&
			wcscmp(data.cFileName, L"..") != 0)
		{
			fullPath.EnsureTrailingSlash();
		}
		_current = std::make_shared<DirectoryEntry>(fullPath);
	}

	void increment() noexcept
	{
		boost::system::error_code ec;
		increment(ec);
	}

	bool equal(const DirectoryContentIterator& other) const
	{
		return _handle == other._handle;
	}

	const DirectoryEntry& dereference() const noexcept
	{
		return *_current;
	}

	// shared_ptr provides shallow-copy semantics required for InputIterators.
	// m_imp.get()==0 indicates the end iterator.
	std::shared_ptr</*HANDLE*/void> _handle;
	const WindowsPath _path;
	std::shared_ptr<DirectoryEntry> _current;
};

// Support C++11 range based loops
inline const DirectoryContentIterator& begin(const DirectoryContentIterator& iter) noexcept
{
	return iter;
}

inline DirectoryContentIterator end(const DirectoryContentIterator&) noexcept
{
	return DirectoryContentIterator();
}


}
}
}
}