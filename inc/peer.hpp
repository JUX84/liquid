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
		std::string* getHexIP();
		void updateStats(unsigned long stats);
		void resetStats();
		std::string record(const unsigned int&, const std::string&);
		static std::string remove(const std::string&, const unsigned int&);
		bool timedOut();
};
