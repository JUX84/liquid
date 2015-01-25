#pragma once

#include <string>
#include <unordered_map>
#include "peers.hpp"

class Torrent {
	private:
		Peers seeders;
		Peers leechers;
		unsigned int id;
		unsigned long size;
		unsigned char free;
		unsigned int downloaded;
	public:
		Torrent(unsigned int, unsigned long, unsigned char, unsigned int);
		Peers* getSeeders();
		Peers* getLeechers();
		unsigned int getDownloaded();
		void downloadedpp();
		unsigned int getID();
		unsigned int getSize();
		unsigned char getFree();
		void setFree(unsigned char);
};
