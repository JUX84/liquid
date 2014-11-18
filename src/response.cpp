#include "response.hpp"
#include "utility.hpp"

std::string response_head (const bool& gzip)
{
	std::string encoding = (gzip ? "\r\nContent-Encoding: gzip" : "");
	return "HTTP/1.1 200 OK\r\nServer: liquid 0.0\r\nContent-Type: text/plain" + encoding + "\r\nConnection: close\r\n\r\n";
}

std::string response (const std::string& body, const bool& gzip)
{
	if (gzip)
		return response_head(gzip) + Utility::gzip_compress(body);
	return response_head(gzip) + body;
}

std::string error (const std::string& reason, const bool& gzip)
{
	return response("d14:failure reason" + std::to_string(reason.size()) + ":" + reason + "12:min intervali5400e8:intervali5400ee", gzip);
}
