#include <cassert>
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include "utility.hpp"

std::string Utility::ip_port_hex_encode (const std::string& ip, const std::string& port)
{
	in_addr addr;
	inet_pton(AF_INET, ip.c_str(), &(addr.s_addr));
	uint16_t value = htons(std::stoi(port));
	return std::string(reinterpret_cast<const char*>(&(addr.s_addr)), sizeof(addr.s_addr)) + std::string(reinterpret_cast<const char*>(&value), sizeof(value));
}

std::string Utility::ip_hex_decode (const std::string& input) {
	char ip[INET_ADDRSTRLEN] = {0};
	std::string tmp(input, 0, 4);

	inet_ntop(AF_INET, tmp.data(), ip, INET_ADDRSTRLEN);
	return ip;
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
