#pragma once

#include "parser.hpp"
#include "torrent.hpp"

typedef std::map<std::string, Torrent*> torrentMap;

class RequestHandler {
	private:
		static torrentMap torMap;
		static std::string announce(const request& req);
	public:
		static std::string handle(std::string str, std::string ip);
};
