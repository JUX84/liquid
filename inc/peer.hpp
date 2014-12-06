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
		long long seedtime;
		long long lastUpdate;
	public:
		Peer (std::string, User*, bool, unsigned int, std::string);
		User* User();
		std::string* getHexIP();
		void updateStats(unsigned long stats, const long long&);
		void resetStats();
		std::string record(const unsigned int&, const std::string&);
		static std::string remove(const std::string&, const unsigned int&);
		bool timedOut(const long long& now);
};
