#pragma once

#include <string>

class Utility
{
	public:
		static std::string ip_port_hex_encode (const std::string&, const std::string&, bool);
		static std::string hex_to_bin (const std::string&);
		static std::string long2ip (unsigned int);
		static std::string gzip_compress (const std::string& input);
		static std::string getPrefix(unsigned char level);
};
