#pragma once

#include <ev++.h>
#include "connectionHandler.hpp"

class Server
{
public:
	Server(uint16_t port);

	void run();
	void acceptClient(ev::io& w, int revents);

private:
	int sock;
	ev::io watcher;
};
