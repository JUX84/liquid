#include <zlib.h>
#include <cassert>
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include "utility.hpp"

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

std::string Utility::ip_hex_encode (const std::string& input)
{
	in_addr addr;
	inet_pton(AF_INET, input.c_str(), &(addr.s_addr));
	return std::string(reinterpret_cast<const char*>(&(addr.s_addr)), sizeof(addr.s_addr));
}

std::string Utility::port_hex_encode (const std::string& input)
{
	uint16_t value = htons(std::stoi(input));
	return std::string(reinterpret_cast<const char*>(&value), sizeof(value));
}

#include <cstdio>

std::string Utility::hex_to_bin(const std::string& input)
{
	std::string output;
	size_t pos;
	size_t size = input.size();
	for (pos = 0; pos < size; pos++) {
		char v = 0;
		if (input[pos] == '%' && size - pos >= 2) {
			for (int i = 4; i >= 0; i -= 4) {
				++pos;
				char c = input[pos];

				if (isdigit(c))
					c = (c - '0') << i;
				else if (c >= 'a' && c <= 'f')
					c = (c - 'a' + 10) << i;
				else
					return "";

				v += c;
			}
		}
		else {
			v = input[pos];
		}

		printf("%x\n", (unsigned char)v);

		output.push_back(v);
	}

	return output;
}
