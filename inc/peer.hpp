#pragma once

#include "user.hpp"

class Peer {
	private:
		User* user;
		unsigned long total_stats;
		unsigned long stats;
		unsigned long left;
		bool seeding;
		bool completed;
		bool active;
		std::string IP;
		std::string hexIPPort;
		std::string peerID;
		unsigned int fid;
		std::string client;
		unsigned long seedtime;
		long long lastUpdate;
		unsigned int speed;
		unsigned int corrupt;
	public:
		Peer (std::string, std::string, User*, bool, unsigned long, unsigned int, std::string, std::string);
		User* getUser();
		const std::string& getPeerID();
		const std::string& getIP();
		const std::string& getHexIPPort();
		const std::string& getClient();
		void updateStats(unsigned long, unsigned long, unsigned int, long long);
		unsigned long getTotalStats();
		unsigned long getStats();
		unsigned long getLeft();
		long long getLastUpdate();
		unsigned int getSpeed();
		unsigned int getFID();
		unsigned int getCorrupt();
		bool isSeeding();
		bool isCompleted();
		bool isActive();
		void inactive();
		bool timedOut(long long);
		bool isSnatched();
		void snatched();
		unsigned long getSeedtime();
		bool hasChanged();
};
