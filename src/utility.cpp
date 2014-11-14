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
	zs.zalloc = NULL;
	zs.zfree = NULL;
	zs.opaque = NULL;
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
	uint16_t value = std::stoi(input);
	return std::to_string((value >> 8)) + std::to_string((value & 0xff));
}
