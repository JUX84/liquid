#pragma once

#include <ev++.h>

class Stats {
	private:
		static bool changed;
		static unsigned long peers;
		static unsigned long torrents;
		static unsigned long speed;
		static double speedShow;
		static unsigned char speedLvl;
		static unsigned long transferred;
		static double transferredShow;
		static unsigned char transferredLvl;
	public:
		static void incPeers();
		static void decPeers();
		static void incTorrents();
		static void decTorrents();
		static void setTorrents(unsigned long);
		static unsigned long getTorrents();
		static void incSpeed(unsigned long);
		static void decSpeed(unsigned long);
		static void incTransferred(unsigned long);
		static void show(ev::timer&, int);
};
