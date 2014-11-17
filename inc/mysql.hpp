#pragma once

#include <mysql.h>
#include "db.hpp"

class MySQL : public Database {
	private:
		MYSQL* mysql;
		MYSQL_RES *result;
		MYSQL_ROW row;
public:
	virtual void Connect();
	virtual void Disconnect()
	virtual void LoadUsers(userMap&);
};
