#include <cassert>
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include "utility.hpp"
#include "config.hpp"

std::string Utility::ip_port_hex_encode (const std::string& ip, const std::string& port)
{
	in_addr addr4;
	/* in6_addr addr6; */
	char* addr = nullptr;
	int af;
	int size;

	/* if (Config::get("ipv6") == "no") { */
		af = AF_INET;
		addr = reinterpret_cast<char*>(&(addr4.s_addr));
		size = sizeof(addr4.s_addr);
	/* } */
	/* else { */
	/* 	af = AF_INET6; */
	/* 	addr = reinterpret_cast<char*>(&(addr6.s6_addr)); */
	/* 	size = sizeof(addr6.s6_addr); */
	/* } */

	inet_pton(af, ip.c_str(), addr);
	uint16_t value = htons(std::stoi(port));
	return std::string(addr, size) + std::string(reinterpret_cast<const char*>(&value), sizeof(value));
}

std::string Utility::hex_to_bin(const std::string& input)
{
	std::string output;
	output.reserve(20);
	size_t pos;
	size_t size = input.size();
	for (pos = 0; pos < size; pos++) {
		char v = 0;
		if (input[pos] == '%' && size - pos >= 2) {
			for (int i = 4; i >= 0; i -= 4) {
				char c = input[++pos];

				if (isdigit(c))
					c = (c - '0') << i;
				else if (c >= 'a' && c <= 'f')
					c = (c - 'a' + 10) << i;
				else if (c >= 'A' && c <= 'F')
					c = (c - 'A' + 10) << i;
				else
					return "";

				v += c;
			}
		}
		else {
			v = input[pos];
		}

		output.push_back(v);
	}

	return output;
}

std::string Utility::long2ip (unsigned int lip) {
	in_addr x;
	x.s_addr = htonl(lip);
	return inet_ntoa(x);
}
