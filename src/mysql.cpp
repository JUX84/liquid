#include <iostream>
#include "config.hpp"
#include "mysql.hpp"

void MySQL::Connect() {
	mysql = mysql_init(nullptr);
	if (!mysql_real_connect(mysql, Config::get("DB_Host").c_str(), Config::get("DB_User").c_str(), Config::get("DB_Password").c_str(), "users", Config::getInt("DB_Port"), Config::get("DB_Socket").c_str(), 0))
		std::cerr << "Couldn't connect to database" << '\n';
}

void MySQL::Disconnect() {
	mysql_free_result(result);
	mysql_close(mysql);
}

void MySQL::LoadUsers(userMap& usrMap) {
	Connect();

	std::string query = "SELECT passkey FROM users";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		usrMap.emplace(row[0], User());
	}

	Disconnect();
}
