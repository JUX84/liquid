#include <system_error>
#include "connectionHandlerIPv6.hpp"

ConnectionHandlerIPv6::ConnectionHandlerIPv6(int socket) : ConnectionHandler(socket)
{
	init();
}

ConnectionHandlerIPv6::~ConnectionHandlerIPv6()
{}

void ConnectionHandlerIPv6::getPeerInfo()
{
	sockaddr_in client;
	socklen_t addrLen = sizeof(client);
	memset(&client, 0, addrLen);

	if (getpeername(sock, reinterpret_cast<sockaddr*>(&client), &addrLen) == -1)
		throw std::system_error(errno, std::system_category());

	binIp = std::string(reinterpret_cast<const char*>(&(client.sin_addr.s_addr)), sizeof(client.sin_addr.s_addr));
}
