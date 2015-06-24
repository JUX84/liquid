#pragma once

#include "connectionHandler.hpp"

class ConnectionHandlerIPv4 : public ConnectionHandler
{
	public:
		ConnectionHandlerIPv4(int socket);
		virtual ~ConnectionHandlerIPv4();

	protected:
		virtual void getPeerInfo();
};
