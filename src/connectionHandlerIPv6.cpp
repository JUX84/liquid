#include <system_error>
#include "connectionHandlerIPv6.hpp"

ConnectionHandlerIPv6::ConnectionHandlerIPv6(int socket) : ConnectionHandler(socket)
{
	ipv6 = true;
	init();
}

ConnectionHandlerIPv6::~ConnectionHandlerIPv6()
{}

void ConnectionHandlerIPv6::getPeerInfo()
{
	char ipBuffer[INET6_ADDRSTRLEN] = {0};
	sockaddr_in6 client;
	socklen_t addrLen = sizeof(client);
	memset(&client, 0, addrLen);

	if (getpeername(sock, reinterpret_cast<sockaddr*>(&client), &addrLen) == -1)
		throw std::system_error(errno, std::system_category());

	if (!inet_ntop(AF_INET6, &(client.sin6_addr.s6_addr), ipBuffer, INET6_ADDRSTRLEN))
		throw std::system_error(errno, std::system_category());

	IP = ipBuffer;
	std::string::size_type pos = IP.find_last_of('.');

	if (pos != std::string::npos) { // IPv4
		ipv6 = false;
		pos = IP.find_last_of(':');
		IP = IP.substr(pos + 1);
	}
}
