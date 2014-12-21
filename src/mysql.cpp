#include <iostream>
#include "logger.hpp"
#include "config.hpp"
#include "mysql.hpp"
#include "user.hpp"
#include "torrent.hpp"
#include "utility.hpp"

void MySQL::connect() {
	mysql = mysql_init(nullptr);
	if (mysql_real_connect(mysql, Config::get("DB_Host").c_str(), Config::get("DB_User").c_str(), Config::get("DB_Password").c_str(), Config::get("DB_DBName").c_str(), Config::getInt("DB_Port"), nullptr, 0) == nullptr) {
		LOG_ERROR("Couldn't connect to database");
	} else {
		LOG_INFO("Succesfully connected to database");
		std::string query = "TRUNCATE xbt_files_users;";
		if (mysql_real_query(mysql, query.c_str(), query.size()))
			return;
	}
}

void MySQL::disconnect() {
	LOG_WARNING("Disconnecting from database");
	mysql_free_result(result);
	mysql_close(mysql);
}

void MySQL::loadUsers(UserMap& usrMap) {
	std::string query = "SELECT passkey, id, nick FROM users";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result)))
		usrMap.emplace(row[0], new User(std::stoul(row[1])));
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " users");

	// load tokens
	query = "SELECT passkey, info_hash FROM users_freeleeches AS uf LEFT JOIN users AS u ON uf.UserID = u.ID JOIN torrents AS t ON uf.TorrentID = t.ID WHERE uf.Expired = '0'";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		try {
			usrMap.at(row[0])->addToken(row[1]);
		} catch (const std::exception& e) {}
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " user tokens");
}

void MySQL::loadTorrents(TorrentMap& torMap) {
	std::string query = "SELECT info_hash, id, name FROM torrents";
	if (mysql_real_query(mysql, query.c_str(), query.size()))
		return;
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result)))
		torMap.emplace(row[0], Torrent(std::stoul(row[1])));
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " torrents");
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
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " banned ips");
}

void MySQL::record (std::string request) {
	requests.push_front(request);
	if (requests.size() > 10)
		flush();
}

void MySQL::flush() {
	LOG_INFO("flushing sql records");
	for(const auto &it : requests) {
		if (mysql_real_query(mysql, it.c_str(), it.size()))
			return;
	}
}
