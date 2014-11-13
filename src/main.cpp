#include <iostream>
#include "server.hpp"
#include "parser.hpp"

int main()
{
	try {
		Server server(48151);
		Parser::init();
		server.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
