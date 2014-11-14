#include <iostream>
#include "server.hpp"
#include "parser.hpp"
#include "config.hpp"

int main()
{
	try {
		Server server(Config::getInt("port"));
		Parser::init();
		server.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
