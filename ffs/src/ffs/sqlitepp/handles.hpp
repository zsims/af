#pragma once

#include <sqlite3.h>

#include <string>

namespace af {
namespace ffs {
namespace sqlitepp {

template <typename T>
class ScopedHandleBase
{
public:
	ScopedHandleBase(const ScopedHandleBase& that) = delete;
	ScopedHandleBase& operator=(const ScopedHandleBase&) = delete;

	ScopedHandleBase()
		: _handle(nullptr)
	{
	}

	explicit ScopedHandleBase(T handle)
		: _handle(handle)
	{
	}

	virtual ~ScopedHandleBase() { };
	operator T() const { return _handle; }
	operator T*() { return &_handle; }
	operator bool() const { return _handle != nullptr; }
protected:
	T _handle;
};

/**
 * Scoped Sqlite3 object (database) as filled out with sqlite3_open_v2
 */
class ScopedSqlite3Object : public ScopedHandleBase<sqlite3*>
{
public:
	~ScopedSqlite3Object() override { sqlite3_close_v2(_handle); }
};

/**
 * Scoped statement as allocated by sqlite3_prepare
 */
class ScopedStatement : public ScopedHandleBase<sqlite3_stmt*>
{
public:
	~ScopedStatement() override { sqlite3_finalize(_handle); }
};

/**
 * Error message buffer as filled by sqlite3_exec, and other similar functions.
 */
class ScopedErrorMessage : public ScopedHandleBase<char*>
{
public:
	~ScopedErrorMessage() override { sqlite3_free(_handle); }
	operator std::string() const { if (_handle != nullptr) { return std::string(_handle); } return std::string(); }
};

}
}
}