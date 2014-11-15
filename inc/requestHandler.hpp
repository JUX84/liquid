#pragma once

#include "parser.hpp"
#include "torrent.hpp"
//#include "user.hpp"

//typedef std::unordered_map<std::string, User*> peerMap;
//typedef std::map<std::string, std::pair<peerMap, peerMap>> torrentMap;
typedef std::map<std::string, Torrent> torrentMap;

class RequestHandler {
	private:
		static torrentMap torMap;
		static std::string announce(const request& req);
	public:
		static std::string handle(std::string str, std::string ip);
};
