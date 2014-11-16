#pragma once

#include <string>
#include <unordered_map>
#include "peermap.hpp"

class Torrent {
	private:
		PeerMap seeders;
		PeerMap leechers;
	public:
		PeerMap* Seeders();
		PeerMap* Leechers();
};
