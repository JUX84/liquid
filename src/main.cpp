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
}

int main()
{
	std::ios_base::sync_with_stdio(false);
	signal(SIGINT, handler);
	signal(SIGTERM, handler);

	try {
		//Config::load("liquid.conf");
		Server server(Config::getInt("port"));
		Parser::init();
		if (Config::get("type") == "private")
			RequestHandler::init();
		server.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
