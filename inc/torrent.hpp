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
		long balance;
		bool changed;
	public:
		Torrent(unsigned int, unsigned long, unsigned char, long);
		Peers* getSeeders();
		Peers* getLeechers();
		unsigned int getSnatches();
		void incSnatches();
		void reset();
		unsigned int getID();
		unsigned int getSize();
		unsigned char getFree();
		void incBalance(unsigned long);
		void decBalance(unsigned long);
		long getBalance();
		void setFree(unsigned char);
		void change();
		bool hasChanged();
};
