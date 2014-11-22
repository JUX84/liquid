#pragma once

#include "user.hpp"

class Peer {
	private:
		User* user;
		unsigned long stats;
		std::string hexIP;
	public:
		Peer (std::string, User*);
		User* User();
		std::string* HexIP();
		void UpdateStats(unsigned long stats);
		void ResetStats();
		std::string Record();
};
