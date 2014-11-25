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
		Peers* getSeeders();
		Peers* getLeechers();
		unsigned int getDownloaded();
		void downloadedpp();
		unsigned int getID();
};
