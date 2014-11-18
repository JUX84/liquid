#pragma once

#include <string>

class Utility
{
	public:
		static std::string gzip_compress (const std::string&);
		static std::string ip_hex_encode(const std::string&);
		static std::string port_hex_encode (const std::string&);
		static std::string hex_to_bin(const std::string&);
};
