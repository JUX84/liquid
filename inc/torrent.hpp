#pragma once

#include <string>
#include <unordered_map>
#include "user.hpp"

typedef std::unordered_map<std::string, User*> peerMap;

class Torrent {
	private:
		peerMap seeders;
		peerMap leechers;
	public:
		peerMap* Seeders();
		peerMap* Leechers();
};
