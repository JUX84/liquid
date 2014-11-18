#pragma once

#include "parser.hpp"
#include "db.hpp"

class RequestHandler {
	private:
		static torrentMap torMap;
		static userMap usrMap;
		static Database *db;
		static std::string announce(const request&);
		static std::string scrape(const std::forward_list<std::string>&, bool);
	public:
		static void init();
		static std::string handle(std::string, std::string);
		static User* getUser(const std::string&);
};
