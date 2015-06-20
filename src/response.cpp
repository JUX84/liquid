#include "logger.hpp"
#include "response.hpp"
#include "utility.hpp"

std::string response_head ()
{
	return "HTTP/1.1 200 OK\r\nServer: liquid 0.0\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n";
}

std::string response (const std::string& body)
{
	return response_head() + body;
}

std::string error (const std::string& reason)
{
	LOG_INFO("Error (" + reason + ")");
	return response("d14:failure reason" + std::to_string(reason.size()) + ":" + reason + "12:min intervali5400e8:intervali5400ee");
}
