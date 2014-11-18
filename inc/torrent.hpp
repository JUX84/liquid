#pragma once

#include <string>
#include <unordered_map>
#include "peers.hpp"

class Torrent {
	private:
		Peers seeders;
		Peers leechers;
	public:
		Peers* Seeders();
		Peers* Leechers();
};
