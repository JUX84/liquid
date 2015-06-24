#include <cassert>
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include <zlib.h>
#include "misc/config.hpp"
#include "misc/utility.hpp"

#define CHUNK 32768

std::string Utility::gzip_compress (const std::string& input)
{
	z_stream zs;
	int ret;
	char out[CHUNK];
	std::string output;
	memset(&zs, 0, sizeof(zs));
	zs.next_in = (Bytef*)input.data();
	zs.zalloc = nullptr;
	zs.zfree = nullptr;
	zs.opaque = nullptr;
	zs.avail_in = input.size();

	deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);

	do {
		zs.next_out = reinterpret_cast<Bytef*>(out);
		zs.avail_out = sizeof(out);

		ret = deflate(&zs, Z_FINISH);

		if (output.size() < zs.total_out)
			output.append(out, zs.total_out - output.size());
	} while (ret == Z_OK);
	deflateEnd(&zs);

	return output;
}

std::string Utility::ip_port_hex_encode (const std::string& ip, const std::string& port, bool ipv6)
{
	in_addr addr4;
	in6_addr addr6;
	char* addr = nullptr;
	int af;
	int size;

	if (ipv6) {
		af = AF_INET6;
		addr = reinterpret_cast<char*>(&(addr6.s6_addr));
		size = sizeof(addr6.s6_addr);
	} else {
		af = AF_INET;
		addr = reinterpret_cast<char*>(&(addr4.s_addr));
		size = sizeof(addr4.s_addr);
	}

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

std::string Utility::getPrefix(unsigned char level) {
	std::string bytes;
	switch (level) {
		case 1:
			bytes = "k";
			break;
		case 2:
			bytes = "M";
			break;
		case 3:
			bytes = "G";
			break;
		case 4:
			bytes = "T";
			break;
		case 5:
			bytes = "P";
			break;
		case 6:
			bytes = "E";
			break;
		case 7:
			bytes = "Z";
			break;
		case 8:
			bytes = "Y";
			break;
		default:
			bytes = "";
			break;
	}
	return bytes;
}
