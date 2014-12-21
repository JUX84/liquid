#pragma once

#include <string>
#include <unordered_map>
#include "peers.hpp"

class Torrent {
	private:
		Peers seeders;
		Peers leechers;
		unsigned int id;
		unsigned char free;
		unsigned int downloaded;
	public:
		Torrent(unsigned int, unsigned char, unsigned int);
		Peers* getSeeders();
		Peers* getLeechers();
		unsigned int getDownloaded();
		void downloadedpp();
		unsigned int getID();
		unsigned char getFree();
		void setFree(unsigned char);
};
