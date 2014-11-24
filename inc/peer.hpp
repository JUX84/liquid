#pragma once

#include "user.hpp"

class Peer {
	private:
		User* user;
		unsigned long stats;
		bool seeding;
		std::string hexIP;
		unsigned int fid;
		std::string client;
		unsigned int lastUpdate;
	public:
		Peer (std::string, User*, bool, unsigned int, std::string);
		User* User();
		std::string* HexIP();
		void UpdateStats(unsigned long stats);
		void ResetStats();
		std::string Record();
};
