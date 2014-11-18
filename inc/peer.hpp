#pragma once

#include "user.hpp"

class Peer {
	private:
		User* user;
		unsigned long downloaded;
		unsigned long uploaded;
	public:
		Peer (User*);
		User* getUser();
};
