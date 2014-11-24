#pragma once

#include <string>
#include <unordered_map>
#include "peers.hpp"

class Torrent {
	private:
		Peers seeders;
		Peers leechers;
		unsigned int downloaded;
		unsigned int id;
	public:
		Torrent(unsigned int);
		Peers* Seeders();
		Peers* Leechers();
		unsigned int Downloaded();
		void Downloadedpp();
		unsigned int GetID();
};
