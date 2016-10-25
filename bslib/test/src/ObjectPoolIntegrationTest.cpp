#include "bslib/ObjectPool.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <chrono>
#include <thread>
#include <vector>

namespace af {
namespace bslib {
namespace test {

namespace {

static bool SampleDeleterCalled = false;

struct SampleResource
{
};

struct SampleDeleter
{
	void operator()(SampleResource* b)
	{
		SampleDeleterCalled = true;
	}
};

}

TEST(ObjectPoolIntegrationTest, AddOne_CallsCreateFn)
{
	// Arrange
	bool called = false;
	ObjectPool<SampleResource> pool(1, [&]() {
		called = true;
		return std::make_unique<SampleResource>();
	});

	// Act
	pool.AddOne();

	// Assert
	EXPECT_TRUE(called);
}

TEST(ObjectPoolIntegrationTest, AddOne_DoesNothingIfAtCapacity)
{
	// Arrange
	int callCount = 0;
	ObjectPool<SampleResource> pool(1, [&]() {
		callCount++;
		return std::make_unique<SampleResource>();
	});
	
	pool.AddOne();

	// Act
	pool.AddOne();

	// Assert
	EXPECT_EQ(1, callCount);
}

TEST(ObjectPoolIntegrationTest, Acquire_CreatesResources)
{
	// Arrange
	int callCount = 0;
	ObjectPool<SampleResource> pool(1, [&]() {
		callCount++;
		return std::make_unique<SampleResource>();
	});
	
	// Act
	pool.Acquire();

	// Assert
	EXPECT_EQ(1, callCount);
}

TEST(ObjectPoolIntegrationTest, Acquire_WaitsInOrder)
{
	using namespace std::chrono_literals;

	// Arrange
	std::vector<int> results;
	ObjectPool<SampleResource> pool(1, [&]() {
		return std::make_unique<SampleResource>();
	});

	// Take the only resource
	auto lockedResource = pool.Acquire();

	// Act
	// Note, no mutex around the results vector is acquired as even though these are
	// are happening in multiple threads, they're running only one thread at a time as a
	// result of the object pool
	std::thread acquire1([&]() {
		pool.Acquire();
		results.push_back(0);
	});

	while (pool.GetWaitCount() != 1)
	{
		std::this_thread::sleep_for(1ms);
	}

	std::thread acquire2([&]() {
		pool.Acquire();
		results.push_back(1); 
	});

	while (pool.GetWaitCount() != 2)
	{
		std::this_thread::sleep_for(1ms);
	}

	// Are now two waiters in line, let things flow to completion
	lockedResource.reset();
	acquire1.join();
	acquire2.join();

	// Assert
	// Ensure the waiters were released in the right order
	EXPECT_THAT(results, testing::ElementsAre(0, 1));
}

TEST(ObjectPoolIntegrationTest, Destructor_CallsOriginalDeleter)
{
	// Arrange
	SampleResource foo;
	SampleDeleterCalled = false;
	{
		ObjectPool<SampleResource, SampleDeleter> pool(1, [&]() {
			return std::unique_ptr<SampleResource, SampleDeleter>(&foo);
		});
		pool.Acquire();
	}
	// Act (dtor)

	// Assert
	ASSERT_TRUE(SampleDeleterCalled);
}

}
}
}
