#include <system_error>
#include "network/connectionHandlerIPv4.hpp"

ConnectionHandlerIPv4::ConnectionHandlerIPv4(int socket) : ConnectionHandler(socket)
{
	ipv6 = false;
	init();
}

ConnectionHandlerIPv4::~ConnectionHandlerIPv4()
{}

void ConnectionHandlerIPv4::getPeerInfo()
{
	char ipBuffer[INET_ADDRSTRLEN] = {0};
	sockaddr_in client;
	socklen_t addrLen = sizeof(client);
	memset(&client, 0, addrLen);

	if (getpeername(sock, reinterpret_cast<sockaddr*>(&client), &addrLen) == -1)
		throw std::system_error(errno, std::system_category());

	if (!inet_ntop(AF_INET, &(client.sin_addr.s_addr), ipBuffer, INET_ADDRSTRLEN))
		throw std::system_error(errno, std::system_category());

	IP = ipBuffer;
}
