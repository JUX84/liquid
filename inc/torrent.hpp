#pragma once

#include <string>
#include <unordered_map>
#include "user.hpp"

typedef std::unordered_map<std::string, User*> peerMap;

class Torrent {
	private:
		peerMap::iterator sit;
		peerMap::iterator lit;
		peerMap seeders;
		peerMap leechers;
	public:
		peerMap::iterator* LastSeeder();
		peerMap::iterator* LastLeecher();
		peerMap* Seeders();
		peerMap* Leechers();
};
