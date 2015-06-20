#pragma once

#include <string>

class Utility
{
	public:
		static std::string ip_port_hex_encode (const std::string&, const std::string&);
		static std::string ip_hex_decode (const std::string&);
		static std::string hex_to_bin (const std::string&);
		static std::string long2ip (unsigned int);
};
