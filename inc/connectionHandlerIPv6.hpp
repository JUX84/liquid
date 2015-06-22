#pragma once

#include "connectionHandler.hpp"

class ConnectionHandlerIPv6 : public ConnectionHandler
{
	public:
		ConnectionHandlerIPv6(int socket);
		virtual ~ConnectionHandlerIPv6();

	protected:
		virtual void getPeerInfo();
};

