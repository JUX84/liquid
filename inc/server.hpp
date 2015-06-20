#pragma once

#include <netinet/in.h>
#include <ev++.h>
#include "connectionHandler.hpp"

class Server
{
	public:
		Server();
		virtual ~Server();
		void run();
		void acceptClient(ev::io&, int);

	protected:
		void init(int domain, uint16_t port, sockaddr* address, socklen_t addrlen);
		virtual void handle(int responseSock) = 0;

	private:
		int sock;
		ev::io watcher;
		ev::timer timer;
		ev::timer timer2;
};
