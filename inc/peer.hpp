#pragma once

#include "user.hpp"

class Peer {
	private:
		User* user;
		unsigned long total_stats;
		unsigned long stats;
		bool seeding;
		std::string hexIP;
		std::string peerID;
		unsigned int fid;
		std::string client;
		long long seedtime;
		long long lastUpdate;
	public:
		Peer (std::string, User*, bool, unsigned int, std::string, std::string);
		User* User();
		std::string* getHexIP();
		void updateStats(unsigned long stats, const long long&);
		void resetStats();
		std::string record(const unsigned int&);
		std::string remove();
		bool timedOut(const long long& now);
		std::string snatch();
};
