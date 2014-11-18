#pragma once

#include "parser.hpp"
#include "db.hpp"

class RequestHandler {
	private:
		static TorrentMap torMap;
		static UserMap usrMap;
		static Database *db;
		static std::string announce(const Request&);
		static std::string scrape(const std::forward_list<std::string>&, bool);
	public:
		static void init();
		static std::string handle(std::string, std::string);
		static User* getUser(const std::string&);
};
