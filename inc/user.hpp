#pragma once

#include <string>
#include <forward_list>
#include <unordered_set>
#include <unordered_map>

class Peer;

class User {
	private:
		unsigned int id;
		unsigned long uploaded;
		unsigned long downloaded;
		bool canLeech;
		bool isVisible;
		long long lastUpdate;
		std::unordered_map<unsigned int, long long> tokens;
		std::unordered_set<std::string> IPRestrictions;
	public:
		User (unsigned int, bool, bool);
		unsigned int getID();
		unsigned long getDownloaded();
		unsigned long getUploaded();
		void reset();
		void updateStats(unsigned int, unsigned int, long long);
		void addToken(unsigned int);
		void removeToken(unsigned int);
		bool hasToken(unsigned int);
		bool isTokenExpired(unsigned int);
		bool canRecord(long long);
		bool addIPRestriction(std::string, int);
		void removeIPRestriction(std::string);
		bool isRestricted(std::string);
};
