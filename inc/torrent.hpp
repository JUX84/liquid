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
		unsigned char free;
		long long lastUpdate;
	public:
		Torrent(unsigned int, const long long&);
		Peers* getSeeders();
		Peers* getLeechers();
		unsigned int getDownloaded();
		void downloadedpp();
		unsigned int getID();
		unsigned char getFree();
		void setFree(unsigned char);
		void setLastUpdate(const long long&);
		long long* getLastUpdate();
};
