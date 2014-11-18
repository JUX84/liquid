#pragma once

#include <mysql.h>
#include "db.hpp"

class MySQL : public Database {
	private:
		MYSQL* mysql;
		MYSQL_RES *result;
		MYSQL_ROW row;
public:
	void Connect(std::string);
	void Disconnect();
	void LoadUsers(userMap&);
	void LoadTorrents(torrentMap&);
};
