#include <system_error>
#include "network/connectionHandlerIPv6.hpp"
#include "network/serverIPv6.hpp"

ServerIPv6::ServerIPv6(uint16_t port, bool ipv6Only) : Server(), IPv6Only(ipv6Only)
{
	sockaddr_in6 address;
	memset(&address, 0, sizeof(address));
	address.sin6_family = PF_INET6;
	address.sin6_port = htons(port);
	address.sin6_addr = in6addr_any;

	init(PF_INET6, port, reinterpret_cast<sockaddr*>(&address), sizeof(address));
}

ServerIPv6::~ServerIPv6()
{}

void ServerIPv6::setSocketOptions() const
{
	Server::setSocketOptions();

	if (IPv6Only) {
		const int opt = 1;

		if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt)) == -1)
			throw std::system_error(errno, std::system_category());
	}
}

void ServerIPv6::handle(int responseSock) const
{
	new ConnectionHandlerIPv6(responseSock);
}
