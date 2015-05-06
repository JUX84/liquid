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
		unsigned int snatches;
		long long lastUpdate;
	public:
		Torrent(unsigned int, unsigned long, unsigned char);
		Peers* getSeeders();
		Peers* getLeechers();
		unsigned int getSnatches();
		void incSnatches();
		void reset();
		unsigned int getID();
		unsigned int getSize();
		unsigned char getFree();
		void setFree(unsigned char);
		bool canRecord(long long);
};
