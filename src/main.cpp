#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include "logger.hpp"
#include "serverIPv4.hpp"
#include "serverIPv6.hpp"
#include "parser.hpp"
#include "config.hpp"
#include "requestHandler.hpp"
#include "utility.hpp"

void handler(int sig)
{
	RequestHandler::stop();
	exit(0);
}

int main(int argc, char **argv)
{
	std::ios_base::sync_with_stdio(false);
	signal(SIGINT, handler);
	signal(SIGTERM, handler);

	std::string configFile = "liquid.conf";
	unsigned int logLevel = Logger::Level::INFO;
	int opt;

	while ((opt = getopt(argc, argv, "c:v:")) != -1) {
		switch (opt) {
			case 'c':
				configFile = optarg;
				break;
			case 'v':
				try {
					logLevel = std::stoul(optarg);
					if (logLevel <= Logger::Level::ERROR)
						break;
				}
				catch (const std::exception& e) {}
			default:
				std::cerr << "Usage: " << argv[0] << " [-c configFile] [-v <0,1,2>]\n";
				return 1;
		}
	}

	try {
		LOG_INIT(static_cast<Logger::Level>(logLevel));
		Config::load(configFile);

		int port = Config::getInt("port");
		std::string ipv6 = Config::get("ipv6");
		Server* server = nullptr;
		if (ipv6 == "no") // factory?
			server = new ServerIPv4(port);
		else
			server = new ServerIPv6(port);

		Parser::init();

		if (Config::get("type") == "private")
			RequestHandler::init();

		LOG_INFO("Starting " + Config::get("type") + " server on port " + std::to_string(port));
		server->run();
		delete server;
	}
	catch (const std::exception& e) {
		LOG_ERROR(e.what());
	}

	return 0;
}
