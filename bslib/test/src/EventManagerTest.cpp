#include "bslib/EventManager.hpp"

#include <vector>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

namespace af {
namespace bslib {
namespace test {

namespace {

struct SampleEvent
{
	SampleEvent(int value)
		: value(value)
	{
	}

	const int value;

	bool operator==(const SampleEvent& rhs) const
	{
		return value == rhs.value;
	}
};

}

TEST(EventManager, PubSub_Success)
{
	// Arrange
	EventManager<SampleEvent> _manager;

	std::vector<SampleEvent> raisedEvents;
	_manager.Subscribe([&](const auto& theEvent) {
		raisedEvents.push_back(theEvent);
	});

	const SampleEvent expectedEvent1(1);
	const SampleEvent expectedEvent2(2);

	// Act
	_manager.Publish(expectedEvent1);
	_manager.Publish(expectedEvent2);

	// Assert
	EXPECT_THAT(raisedEvents, ::testing::ElementsAre(expectedEvent1, expectedEvent2));
}

}
}
}
