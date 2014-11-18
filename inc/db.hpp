#pragma once

#include <map>
#include "user.hpp"

typedef std::map<std::string, User> userMap;

class Database {
	public:
		virtual void Connect(std::string) {};
		virtual void Disconnect() {};
		virtual void LoadUsers(userMap&) {};
};
