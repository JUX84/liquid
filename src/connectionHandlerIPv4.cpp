#include <system_error>
#include "connectionHandlerIPv4.hpp"

ConnectionHandlerIPv4::ConnectionHandlerIPv4(int socket) : ConnectionHandler(socket)
{
	init();
}

ConnectionHandlerIPv4::~ConnectionHandlerIPv4()
{}

void ConnectionHandlerIPv4::getPeerInfo()
{
	sockaddr_in client;
	socklen_t addrLen = sizeof(client);
	memset(&client, 0, addrLen);

	if (getpeername(sock, reinterpret_cast<sockaddr*>(&client), &addrLen) == -1)
		throw std::system_error(errno, std::system_category());

	binIp = std::string(reinterpret_cast<const char*>(&(client.sin_addr.s_addr)), sizeof(client.sin_addr.s_addr));
}
