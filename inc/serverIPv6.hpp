#pragma once

#include "server.hpp"

class ServerIPv6 : public Server
{
	public:
		ServerIPv6(uint16_t port);
		virtual ~ServerIPv6();

	protected:
		virtual void handle(int responseSock) override;
};
