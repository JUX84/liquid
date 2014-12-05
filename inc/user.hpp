#pragma once

#include <string>
#include <forward_list>

class Peer;

class User {
	private:
		unsigned int id;
		unsigned long uploaded;
		unsigned long downloaded;
	public:
		User (unsigned int id);
		unsigned int* getID();
		void updateStats(unsigned int, unsigned int);
		std::string record();
};
