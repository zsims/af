#include "bs_daemon_lib/HttpServer.hpp"

#include "bslib_test_util/TestBase.hpp"
#include "bslib_test_util/mocks/MockBackup.hpp"
#include "bslib_test_util/mocks/MockUnitOfWork.hpp"

// Work around https://svn.boost.org/trac/boost/ticket/11599
#pragma warning( push )
#pragma warning( disable : 4715)
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#pragma warning( pop )

// Avoid size_t <-> int64 warnings on Windows x32
#pragma warning( push )
#pragma warning( disable : 4244 )
#include <client_http.hpp>
#pragma warning( pop )

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <random>

typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

namespace {
std::random_device randomDevice;
std::mt19937 random(randomDevice());
std::uniform_int_distribution<int> randomUniformDistribution(45000, 46000);

int FindFreePort()
{
	// It's not possible to call bind(address, 0) with the simple http server as it doesn't expose the port it binds to
	return randomUniformDistribution(random);
}

}

namespace af {
namespace bs_daemon {
namespace test {

class HttpServerIntegrationTest : public ::testing::Test
{
protected:
	HttpServerIntegrationTest()
		: _testPort(FindFreePort())
		, _testAddress("127.0.0.1:" + std::to_string(_testPort))
		, _jobExecutor(_mockBackup)
		, _httpServer(_testPort, _jobExecutor)
	{
		ON_CALL(_mockBackup, CreateUnitOfWork())
			.WillByDefault(::testing::Invoke([]() {
			return std::make_unique<bslib_test_util::mocks::MockUnitOfWork>();
		}));
	}
	bslib_test_util::mocks::MockBackup _mockBackup;
	const int _testPort;
	const std::string _testAddress;
	JobExecutor _jobExecutor;
	HttpServer _httpServer;
};

TEST_F(HttpServerIntegrationTest, UnregisteredEndpoints_Return404)
{
	// Arrange
	HttpClient client(_testAddress);

	// Act
	auto response = client.request("GET", "/match/123");

	// Assert
	EXPECT_EQ(response->status_code, "404 Not Found");
}

TEST_F(HttpServerIntegrationTest, Ping_Success)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto rawContent = u8R"({"fancy":"Сука Блять"})";
	const auto expectedFancy = u8"Сука Блять";

	// Act
	auto response = client.request("POST", "/ping", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "200 OK");
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(response->content, pt);
	auto value = pt.get_optional<std::string>("fancy");
	ASSERT_TRUE(value);
	EXPECT_EQ(expectedFancy, value.get());
}

}
}
}
