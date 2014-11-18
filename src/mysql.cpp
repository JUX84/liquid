#include <iostream>
#include "config.hpp"
#include "mysql.hpp"
#include "user.hpp"
#include "torrent.hpp"

void MySQL::Connect() {
	mysql = mysql_init(nullptr);
	if (mysql_real_connect(mysql, Config::get("DB_Host").c_str(), Config::get("DB_User").c_str(), Config::get("DB_Password").c_str(), Config::get("DB_DBName").c_str(), Config::getInt("DB_Port"), nullptr, 0) == nullptr)
		std::cerr << "Couldn't connect to database" << '\n';
}

void MySQL::Disconnect() {
	mysql_free_result(result);
	mysql_close(mysql);
}

void MySQL::LoadUsers(UserMap& usrMap) {
	Connect();

	std::string query = "SELECT passkey, nick FROM users";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result)))
		usrMap.emplace(row[0], User());
	std::cout << "Loaded " << mysql_num_rows(result) << " users\n";

	Disconnect();
}

void MySQL::LoadTorrents(TorrentMap& torMap) {
	Connect();

	std::string query = "SELECT info_hash, name FROM torrents";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result)))
		torMap.emplace(row[0], Torrent());
	std::cout << "Loaded " << mysql_num_rows(result) << " torrents\n";

	Disconnect();
}
