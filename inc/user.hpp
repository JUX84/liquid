#pragma once

#include <string>
#include <forward_list>

class Peer;

class User {
	private:
		unsigned int id;
		unsigned long uploaded;
		unsigned long downloaded;
		bool canLeech;
		bool isVisible;
		long long lastUpdate;
		std::forward_list<std::string> tokens;
	public:
		User (unsigned int, bool, bool);
		unsigned int getID();
		unsigned long getDownloaded();
		unsigned long getUploaded();
		void reset();
		void updateStats(unsigned int, unsigned int, long long);
		std::string record();
		void addToken(const std::string&);
		void removeToken(const std::string&);
		bool hasToken(const std::string&);
		bool canRecord(long long);
};
