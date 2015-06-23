#pragma once

#include <string>

class Stats {
	private:
		static unsigned long peers;
		static double speed;
		static unsigned char speedLvl;
		static double transferred;
		static unsigned char transferredLvl;
	public:
		static void incPeers();
		static void decPeers();
		static std::string getPeers();
		static void incSpeed(unsigned long speed);
		static void decSpeed(unsigned long speed);
		static std::string getSpeed();
		static void incTransferred(unsigned long transferred);
		static std::string getTransferred();

};
