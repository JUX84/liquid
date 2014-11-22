#pragma once

#include <string>
#include <unordered_map>
#include "peers.hpp"

class Torrent {
	private:
		Peers seeders;
		Peers leechers;
		unsigned int downloaded = 0;
	public:
		Peers* Seeders();
		Peers* Leechers();
		unsigned int Downloaded();
		void Downloadedpp();
};
