#pragma once

#include <string>

class Stats {
	private:
		static bool changed;
		static unsigned long peers;
		static unsigned long torrents;
		static double speed;
		static unsigned char speedLvl;
		static double transferred;
		static unsigned char transferredLvl;
	public:
		static void reset();
		static bool hasChanged();
		static void incPeers();
		static void decPeers();
		static std::string getPeers();
		static void incTorrents();
		static void decTorrents();
		static void setTorrents(unsigned long);
		static unsigned long getTorrents();
		static std::string getTorrentsStr();
		static void incSpeed(unsigned long);
		static void decSpeed(unsigned long);
		static std::string getSpeed();
		static void incTransferred(unsigned long);
		static std::string getTransferred();

};
