#pragma once

#include "user.hpp"

class Peer {
	private:
		User* user;
		unsigned long total_stats;
		unsigned long stats;
		bool seeding;
		bool completed;
		bool active;
		std::string hexIP;
		std::string peerID;
		unsigned int fid;
		std::string client;
		long long seedtime;
		long long lastUpdate;
	public:
		Peer (std::string, User*, bool, unsigned int, std::string, std::string);
		User* User();
		const std::string& getPeerID();
		const std::string& getHexIP();
		const std::string& getClient();
		void updateStats(unsigned long stats, long long);
		unsigned long getTotalStats();
		unsigned long getStats();
		long long getLastUpdate();
		unsigned int getFID();
		bool isSeeding();
		bool isCompleted();
		bool isActive();
		void inactive();
		bool timedOut(long long);
		bool isSnatched();
		void snatched();
		void setSeedtime(long long);
		long long getSeedtime();
		void reset(long long);
};
