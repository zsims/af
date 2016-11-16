#include "bs_daemon_lib/HttpServer.hpp"

#include "bslib_test_util/TestBase.hpp"

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

class HttpServerIntegrationTest : public bslib_test_util::TestBase
{
protected:
	HttpServerIntegrationTest()
		: _testPort(FindFreePort())
		, _testAddress("127.0.0.1:" + std::to_string(_testPort))
		, _backup(_testBackup.OpenOrCreate())
		, _jobExecutor(_backup)
		, _httpServer(_testPort, _testBackup.GetBlobStoreManager(), _jobExecutor)
	{
	}
	const int _testPort;
	const std::string _testAddress;
	bslib::Backup& _backup;
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
	auto response = client.request("POST", "/api/ping?foo=bar&fizz=buzz", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "200 OK");
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(response->content, pt);
	auto value = pt.get_optional<std::string>("fancy");
	ASSERT_TRUE(value);
	EXPECT_EQ(expectedFancy, value.get());
	// TODO: the HttpClient doesn't sent he port in the Host: header
	// EXPECT_EQ(_testAddress, pt.get<std::string>("_url.authority"));
	// EXPECT_EQ(std::to_string(_testPort), pt.get<std::string>("_url.port"));
	EXPECT_EQ("127.0.0.1", pt.get<std::string>("_url.host"));
	EXPECT_EQ("/api/ping", pt.get<std::string>("_url.path"));
	EXPECT_EQ("foo=bar&fizz=buzz", pt.get<std::string>("_url.query"));
}

TEST_F(HttpServerIntegrationTest, Ping_BadRequestOnInvalidJson)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto rawContent = u8R"({"notvalid})";

	// Act
	auto response = client.request("POST", "/api/ping", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "400 Bad Request");
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(response->content, pt);
	auto value = pt.get_optional<std::string>("error");
	EXPECT_TRUE(value);
}

TEST_F(HttpServerIntegrationTest, PostFilePath_Success)
{
	// Arrange
	HttpClient client(_testAddress);
	
	const auto testPath = GetUniqueTempPath();
	WriteFile(testPath);

	boost::property_tree::ptree pt;
	pt.push_back(boost::property_tree::ptree::value_type("path", testPath.string()));
	std::stringstream ss;
	boost::property_tree::write_json(ss, pt);
	const auto rawContent = ss.str();

	// Act
	auto response = client.request("POST", "/file", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "202 Accepted");
}

TEST_F(HttpServerIntegrationTest, PostFilePath_BadRequestIfMissingPath)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto rawContent = u8R"({"notpath":"haha"})";

	// Act
	auto response = client.request("POST", "/file", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "400 Bad Request");
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(response->content, pt);
	auto value = pt.get_optional<std::string>("error");
	EXPECT_TRUE(value);
}

TEST_F(HttpServerIntegrationTest, PostStores_Success)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto testPath = GetUniqueTempPath().string();

	boost::property_tree::ptree pt;
	pt.add("type", "directory");
	pt.add("path", testPath);
	std::stringstream ss;
	boost::property_tree::write_json(ss, pt);
	const auto rawContent = ss.str();

	// Act
	auto response = client.request("POST", "/api/stores", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "201 Created");
	boost::property_tree::ptree responseContent;
	boost::property_tree::read_json(response->content, responseContent);
	EXPECT_EQ("directory", responseContent.get<std::string>("type"));
	EXPECT_NE(responseContent.find("id"), responseContent.not_found());
	EXPECT_EQ(testPath, responseContent.get<std::string>("path"));
}

TEST_F(HttpServerIntegrationTest, PostStores_BadRequestIfMissingType)
{
	// Arrange
	HttpClient client(_testAddress);
	const auto rawContent = u8R"({"nottype":"haha"})";

	// Act
	auto response = client.request("POST", "/api/stores", rawContent);

	// Assert
	ASSERT_EQ(response->status_code, "400 Bad Request");
	boost::property_tree::ptree pt;
	boost::property_tree::read_json(response->content, pt);
	auto value = pt.get_optional<std::string>("error");
	EXPECT_TRUE(value);
}

}
}
}
