#include "serverIPv6.hpp"
#include "connectionHandlerIPv6.hpp"

ServerIPv6::ServerIPv6(uint16_t port) : Server()
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

void ServerIPv6::handle(int responseSock)
{
	new ConnectionHandlerIPv6(responseSock);
}
