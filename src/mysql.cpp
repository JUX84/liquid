#include "config.hpp"
#include "mysql.hpp"

void MySQL::Connect() {
	mysql = mysql_init(nullptr);
	if (!mysql_real_connect(mysql, Config::get("DB_Host"), Config::get("DB_User"), Config::get("DB_Password"), "users", Config::get("DB_Port"), Config::get("DB_Socket"), 0))
		std::cerr << "Couldn't connect to database" << std::endl;
}

void MySQL::Disconnect() {
	mysql_free_result(result);
	mysql_close(mysql);
}

void MySQL::LoadUsers(userMap& usrMap) {
	Connect();

	std::string query = "SELECT passkey FROM users";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return 1;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		usrMap.emplace(row[0], User());
	}

	Disconnect();
}
