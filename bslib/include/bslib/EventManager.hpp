#pragma once

#include <functional>
#include <vector>

namespace af {
namespace bslib {

/**
 * Manages publication and subscription of events
 */
template <typename T>
class EventManager
{
public:
	typedef std::function<void(const T& theEvent)> EmitEventFn;

	void Subscribe(EmitEventFn callback)
	{
		_subscribers.push_back(callback);
	}

	void Publish(const T& theEvent) const
	{
		for (const auto& subscriber : _subscribers)
		{
			subscriber(theEvent);
		}
	}
private:
	std::vector<EmitEventFn> _subscribers;
};

}
}
