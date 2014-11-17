#pragma once

#include "requestHandler.hpp"

class Database {
	public:
		virtual void Connect() {};
		virtual void Disconnect() {};
	virtual void LoadUsers(userMap&) {};
};
