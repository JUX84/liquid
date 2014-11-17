#pragma once

#include "parser.hpp"
#include "torrent.hpp"

typedef std::map<std::string, Torrent> torrentMap;
typedef std::map<std::string, User> userMap;

class RequestHandler {
	private:
		static torrentMap torMap;
		static userMap usrMap;
		static std::string announce(const request&);
	public:
		static std::string handle(std::string, std::string);
		static User* getUser(const std::string&);
};
