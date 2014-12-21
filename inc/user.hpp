#pragma once

#include <string>
#include <forward_list>

class Peer;

class User {
	private:
		unsigned int id;
		unsigned long uploaded;
		unsigned long downloaded;
		std::forward_list<std::string> tokens;
	public:
		User (unsigned int id);
		unsigned int* getID();
		void updateStats(unsigned int, unsigned int);
		std::string record();
		void addToken(const std::string&);
		void removeToken(const std::string&);
		bool hasToken(const std::string&);
};
