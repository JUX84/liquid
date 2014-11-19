#pragma once

#include "user.hpp"

class Peer {
	private:
		User* user;
		//unsigned long downloaded;
		//unsigned long uploaded;
		std::string hexIP;
	public:
		Peer (std::string, User*);
		User* getUser();
		std::string* getHexIP();
};
