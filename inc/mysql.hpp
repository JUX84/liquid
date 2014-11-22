#pragma once

#include <mysql.h>
#include "db.hpp"

class MySQL : public Database {
	private:
		MYSQL* mysql;
		MYSQL_RES *result;
		MYSQL_ROW row;
	public:
		void Connect();
		void Disconnect();
		void LoadUsers(UserMap&);
		void LoadTorrents(TorrentMap&);
		void Record(std::string);
		void Flush();
};
