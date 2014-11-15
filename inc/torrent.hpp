#pragma once

#include <string>
#include <unordered_map>
#include "user.hpp"

typedef std::unordered_map<std::string, User*> peerMap;

class Torrent {
	private:
		peerMap::iterator it;
		peerMap seeders;
		peerMap leechers;
	public:
		void LastSeeder(peerMap::iterator it);
		peerMap::iterator LastSeeder();
		peerMap* Seeders();
		peerMap* Leechers();
};
