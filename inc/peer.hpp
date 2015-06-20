#pragma once

#include "user.hpp"

class Peer {
	private:
		User* user;
		unsigned long downloaded;
		unsigned long uploaded;
		unsigned long totalDownloaded;
		unsigned long totalUploaded;
		unsigned int downSpeed;
		unsigned int upSpeed;
		unsigned long left;
		bool seeding;
		bool completed;
		bool active;
		std::string IP;
		std::string hexIPPort;
		std::string peerID;
		unsigned int torrentID;
		std::string client;
		unsigned long timespent;
		long long lastUpdate;
		unsigned int corrupt;
	public:
		Peer (std::string, std::string, User*, bool, unsigned long, unsigned long, unsigned long, unsigned int, std::string, std::string);
		User* getUser();
		const std::string& getPeerID();
		const std::string& getIP();
		const std::string& getHexIPPort();
		const std::string& getClient();
		void updateStats(unsigned long, unsigned long, unsigned long, unsigned int, long long);
		unsigned long getTotalDownloaded();
		unsigned long getDownloaded();
		unsigned long getTotalUploaded();
		unsigned long getUploaded();
		unsigned int getDownSpeed();
		unsigned int getUpSpeed();
		unsigned long getLeft();
		long long getLastUpdate();
		unsigned int getTorrentID();
		unsigned int getCorrupt();
		bool isSeeding();
		bool isCompleted();
		bool isActive();
		void inactive();
		bool timedOut(long long);
		bool isSnatched();
		void snatched();
		unsigned long getTimespent();
};
