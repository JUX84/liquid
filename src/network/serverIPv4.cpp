#include "network/connectionHandlerIPv4.hpp"
#include "network/serverIPv4.hpp"

ServerIPv4::ServerIPv4(uint16_t port) : Server()
{
	sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = PF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	init(PF_INET, port, reinterpret_cast<sockaddr*>(&address), sizeof(address));
}

ServerIPv4::~ServerIPv4()
{}

void ServerIPv4::handle(int responseSock) const
{
	new ConnectionHandlerIPv4(responseSock);
}
