#pragma once

#include "bslib/log.hpp"

#include <memory>
#include <mutex>
#include <queue>
#include <stack>
#include <vector>

#include <boost/noncopyable.hpp>

namespace af {
namespace bslib {

/**
 * Simple object pool that lazily maintains a pool of objects and notifies waiters in order
 */
template<class T, class D = std::default_delete<T>>
class ObjectPool : public boost::noncopyable
{
public:
	struct ReturnToPoolDeleter
	{
		explicit ReturnToPoolDeleter(ObjectPool& owner)
			: _owner(owner)
		{
		}
		void operator()(T* ptr)
		{
			_owner.Return(ptr);
		}
	private:
		ObjectPool& _owner;
	};

	ObjectPool(unsigned capacity, std::function<std::unique_ptr<T, D>()> createFn)
		: _capacity(capacity)
		, _size(0)
		, _createFn(createFn)
	{
		BSLIB_LOG_TRACE << "Object pool created with capacity of " << capacity;
	}

	/**
	 * Adds an object to the pool
	 */
	void AddOne()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (_size < _capacity)
		{
			auto obj = _createFn();
			_available.push(std::move(obj));
			_size++;
			BSLIB_LOG_TRACE << "Object pool preallocated 1 item";
			NotifyWaiters();
		}
	}

	/**
	 * Acquires a single resource, creating it with the previously specified create function
	 */
	std::unique_ptr<T, ReturnToPoolDeleter> Acquire()
	{
		std::unique_lock<std::mutex> lock(_mutex);

		// Free resources and no waiters
		if (_waiters.empty() && !_available.empty())
		{
			BSLIB_LOG_TRACE << "Object pool immediate acquire";
			auto top = std::move(_available.top());
			_available.pop();
			return std::unique_ptr<T, ReturnToPoolDeleter>(top.release(), ReturnToPoolDeleter(*this));
		}

		// No immediately free resources
		if (_size < _capacity)
		{
			BSLIB_LOG_TRACE << "Object pool factory acquire";
			auto obj = _createFn();
			_size++;
			return std::unique_ptr<T, ReturnToPoolDeleter>(obj.release(), ReturnToPoolDeleter(*this));
		}

		// Wait for a resource
		std::condition_variable waiter;
		// Pushing a pointer here is fine, as this thread will remain alive for the duration of the waiter
		BSLIB_LOG_TRACE << "Object pool waiting for available object";
		_waiters.push(&waiter);
		waiter.wait(lock, [&]() {
			return !_available.empty();
		});

		auto top = std::move(_available.top());
		_available.pop();
		return std::unique_ptr<T, ReturnToPoolDeleter>(top.release(), ReturnToPoolDeleter(*this));
	}

	unsigned GetWaitCount()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		return static_cast<unsigned>(_waiters.size());
	}
private:
	void Return(T* obj)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		BSLIB_LOG_TRACE << "Object pool had object returned";
		_available.push(std::unique_ptr<T, D>(obj));
		NotifyWaiters();
	}

	void NotifyWaiters()
	{
		// This assumes it's covered by a lock
		if (!_waiters.empty())
		{
			auto front = _waiters.front();
			_waiters.pop();
			front->notify_one();
		}
	}
private:
	std::mutex _mutex;
	const unsigned _capacity;
	unsigned _size;
	std::function<std::unique_ptr<T, D>()> _createFn;
	std::stack<std::unique_ptr<T, D>> _available;
	std::queue<std::condition_variable*> _waiters;
};

}
}