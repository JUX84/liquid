#pragma once

#include <mysql.h>
#include "db.hpp"

class MySQL : public Database {
	private:
		MYSQL* mysql;
		MYSQL_RES *result;
		MYSQL_ROW row;
	public:
		void connect();
		void disconnect();
		void loadUsers(UserMap&);
		void loadTorrents(TorrentMap&);
		void record(std::string);
		void flush();
};
