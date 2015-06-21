#pragma once

#include "server.hpp"

class ServerIPv4 : public Server
{
	public:
		ServerIPv4(uint16_t port);
		virtual ~ServerIPv4();

	protected:
		virtual void handle(int responseSock) const override;
};
