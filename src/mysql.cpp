#include <iostream>
#include "config.hpp"
#include "mysql.hpp"
#include "user.hpp"
#include "torrent.hpp"
#include "utility.hpp"

void MySQL::connect() {
	mysql = mysql_init(nullptr);
	if (mysql_real_connect(mysql, Config::get("DB_Host").c_str(), Config::get("DB_User").c_str(), Config::get("DB_Password").c_str(), Config::get("DB_DBName").c_str(), Config::getInt("DB_Port"), nullptr, 0) == nullptr)
		std::cerr << "Couldn't connect to database" << '\n';
	else {
		std::cout << "Succesfully connected to database\n";
		std::string query = "TRUNCATE xbt_files_users;";
		if (mysql_real_query(mysql, query.c_str(), query.size()))
			return;
	}
}

void MySQL::disconnect() {
	std::cout << "Disconnecting from database\n";
	mysql_free_result(result);
	mysql_close(mysql);
}

void MySQL::loadUsers(UserMap& usrMap) {
	std::string query = "SELECT passkey, id, nick FROM users";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		usrMap.emplace(row[0], new User(std::stoul(row[1])));
		std::cout << "Loaded user " << row[2] << " (" << row[1] << ") with passkey: " << row[0] << '\n';
	}
	std::cout << "Loaded " << mysql_num_rows(result) << " users\n";
}

void MySQL::loadTorrents(TorrentMap& torMap) {
	std::string query = "SELECT info_hash, id, name FROM torrents";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		torMap.emplace(row[0], Torrent(std::stoul(row[1])));
		std::cout << "Loaded torrent " << row[2] << " (" << row[1] << ") with info_hash: " << row[0] << '\n';
	}
	std::cout << "Loaded " << mysql_num_rows(result) << " torrents\n";
}

void MySQL::loadBannedIps(std::forward_list<std::string> &banned_ips) {
	std::string query = "SELECT FromIP, ToIP FROM ip_bans";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		unsigned int from = std::stoul(row[0]);
		unsigned int to = std::stoul(row[1]);

		while (from != to)
			banned_ips.push_front(Utility::long2ip(from++));
		banned_ips.push_front(Utility::long2ip(from));
		std::cout << "Loaded banned ip (" << row[0] << " to " << row[1] << ")" << '\n';
	}
	std::cout << "Loaded " << mysql_num_rows(result) << " banned ips\n";
}

void MySQL::record (std::string request) {
	std::cout << "Pushed a new record: " << request << '\n';
	requests.push_front(request);
	if (requests.size() > 10)
		flush();
}

void MySQL::flush() {
	std::cout << "flushing records" << '\n';
	for(const auto &it : requests) {
		if (mysql_real_query(mysql, it.c_str(), it.size()))
			return;
	}
}
