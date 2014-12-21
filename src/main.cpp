#include <iostream>
#include <cstdlib>
#include <signal.h>
#include "server.hpp"
#include "parser.hpp"
#include "config.hpp"
#include "requestHandler.hpp"

void handler(int sig)
{
	RequestHandler::stop();
	exit(0);
}

int main()
{
	std::ios_base::sync_with_stdio(false);
	signal(SIGINT, handler);
	signal(SIGTERM, handler);

	try {
		//Config::load("liquid.conf");
		int port = Config::getInt("port");
		Server server(port);
		Parser::init();
		if (Config::get("type") == "private")
			RequestHandler::init();
		LOG_INFO("Starting " + Config::get("type") + " server on port " + std::to_string(port));
		server.run();
	}
	catch (const std::exception& e) {
		LOG_ERROR(e.what());
	}

	return 0;
}
