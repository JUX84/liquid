#pragma once

#include <ev++.h>
#include "connectionHandler.hpp"

class Server
{
	public:
		Server(uint16_t);
		void run();
		void acceptClient(ev::io&, int);
	private:
		int sock;
		ev::io watcher;
};
