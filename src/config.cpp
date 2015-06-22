#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <iterator>
#include <utility>
#include <stdexcept>
#include "logger.hpp"
#include "config.hpp"

std::unordered_map<std::string, std::pair<std::string, bool>> Config::vars = {
//	{ "name",							{ "default value", is_integer} }
	{ "type",							{ "private", false } },
	{ "ipv6",						{ "no", false } },
	{ "port",							{ "2710", true } },
	{ "timeout",						{ "3600", true } },
	{ "clear_peers_interval",			{ "600", true } },
	{ "flush_records_interval",			{ "5", true } },
	{ "read_buffer_size",				{ "512", true } },
	{ "max_request_size",				{ "4096", true } },
	{ "max_numwant",					{ "50", true } },
	{ "default_numwant",				{ "20", true } },
	{ "default_announce_interval",		{ "900", true } },
	{ "min_announce_interval",			{ "300", true } },
	{ "updatekey",						{ "00000000000000000000000000000000", false } },
	{ "DB_Host",						{ "127.0.0.1", false } },
	{ "DB_User",						{ "db_user", false } },
	{ "DB_Password",					{ "db_password", false } },
	{ "DB_DBName",						{ "db_name", false } },
	{ "DB_Port",						{ "3306", true } },
	{ "DB_Users",						{ "users_main", false } },
		{ "DB_Users_ID",				{ "ID", false } },
		{ "DB_Users_Passkey",			{ "torrent_pass", false } },
		{ "DB_Users_Authorized",		{ "can_leech", false } },
		{ "DB_Users_Downloaded",		{ "Downloaded", false } },
		{ "DB_Users_Uploaded",			{ "Uploaded", false } },
	{ "DB_Torrents",					{ "torrents", false } },
		{ "DB_Torrents_ID",				{ "ID", false } },
		{ "DB_Torrents_InfoHash",		{ "info_hash", false } },
		{ "DB_Torrents_Freeleech",		{ "FreeTorrent", false } },
		{ "DB_Torrents_Seeders",		{ "Seeders", false } },
		{ "DB_Torrents_Leechers",		{ "Leechers", false } },
		{ "DB_Torrents_Snatches",		{ "Snatched", false } },
		{ "DB_Torrents_Balance",		{ "balance", false } },
		{ "DB_Torrents_LastAction",		{ "last_action", false } },
	{ "DB_Peers",						{ "xbt_files_users", false } },
		{ "DB_Peers_UserID",			{ "uid", false } },
		{ "DB_Peers_Active",			{ "active", false } },
		{ "DB_Peers_AnnouncesCount",	{ "announced", false } },
		{ "DB_Peers_Completed",			{ "completed", false } },
		{ "DB_Peers_Downloaded",		{ "downloaded", false } },
		{ "DB_Peers_Uploaded",			{ "uploaded", false } },
		{ "DB_Peers_Left",				{ "remaining", false } },
		{ "DB_Peers_UpSpeed",			{ "upspeed", false } },
		{ "DB_Peers_DownSpeed",			{ "downspeed", false } },
		{ "DB_Peers_Corrupt",			{ "corrupt", false } },
		{ "DB_Peers_Timespent",			{ "timespent", false } },
		{ "DB_Peers_UserAgent",			{ "useragent", false } },
		{ "DB_Peers_PeerID",			{ "peer_id", false } },
		{ "DB_Peers_TorrentID",			{ "fid", false } },
		{ "DB_Peers_IP",				{ "ip", false } },
		{ "DB_Peers_LastAction",		{ "mtime", false } },
	{ "DB_Snatches",					{ "xbt_snatched", false } },
		{ "DB_Snatches_UserID",			{ "uid", false } },
		{ "DB_Snatches_TimeStamp",		{ "tstamp", false } },
		{ "DB_Snatches_TorrentID",		{ "fid", false } },
		{ "DB_Snatches_IP",				{ "IP", false } },
	{ "DB_Tokens",						{ "users_freeleeches", false } },
		{ "DB_Tokens_UserID",			{ "UserID", false } },
		{ "DB_Tokens_TorrentID",		{ "TorrentID", false } },
		{ "DB_Tokens_Downloaded",		{ "Downloaded", false } },
		{ "DB_Tokens_ExpirationTime",	{ "Time", false } },
		{ "DB_Tokens_Expired",			{ "Expired", false } },
	{ "DB_IPRestrictions",				{ "tk_ip_restrictions", false } },
		{ "DB_IPRestrictions_UserID",	{ "UserID", false } },
		{ "DB_IPRestrictions_IP",		{ "IP", false } },
	{ "DB_IPBans",						{ "tk_ip_bans", false } },
		{ "DB_IPBans_FromIP",			{ "FromIP", false } },
		{ "DB_IPBans_ToIP",				{ "ToIP", false } },
	{ "DB_Whitelist",					{ "xbt_client_whitelist", false } },
		{ "DB_Whitelist_PeerID",		{ "peer_id", false } },
	{ "DB_SiteOptions",					{ "site_options", false } },
		{ "DB_SiteOptions_LeechStatus",	{ "leech_status", false } },
};

std::string Config::get(const std::string& name)
{
	try {
		return vars.at(name).first;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Error in Config::get(" + name + ")");
		return "";
	}
}

int Config::getInt(const std::string& name)
{
	try {
		return std::stoi(vars.at(name).first);
	}
	catch (const std::exception& e) {
		LOG_ERROR("Error in Config::getInt(" + name + ")");
		return -1;
	}
}

void Config::load(const std::string& file)
{
	std::ifstream f(file);
	std::string line;
	unsigned int lineNumber = 1;
	std::string whitespaces(" \t\v\f\r");
	auto trim = [&whitespaces](const std::string& str, size_t start, size_t end) -> std::string {
		start = str.find_first_not_of(whitespaces, start);
		end = str.find_last_not_of(whitespaces, end);
		return std::string(str, start, end - start + 1);
	};

	if (!f) {
		LOG_WARNING("Couldn't open " + file + ". Aborted.");
		return;
	}

	while (!f.eof()) {
		std::getline(f, line);

		size_t start = line.find_first_not_of(whitespaces);
		if (line.empty() || start == std::string::npos || line[start] == '#') {
			++lineNumber;
			continue; // empty line or comment
		}

		size_t equalSign = line.find('=');
		if (equalSign == std::string::npos || equalSign == start || equalSign == line.size() - 1) {
			LOG_ERROR(std::to_string(lineNumber) + ": syntax error");
			continue;
		}

		std::string name = trim(line, start, equalSign - 1);
		if (vars.find(name) == vars.end()) {
			LOG_ERROR(std::to_string(lineNumber) + ": " + name + ": name not valid");
			continue;
		}
		std::string value = trim(line, equalSign + 1, line.size() - 1);

		if (checkValue(name, value))
			vars.at(name).first = value;

		++lineNumber;
	}
}

bool Config::checkValue(const std::string& name, std::string& value)
{
	if (vars.at(name).second) {
		try {
			std::stoi(value);
		}
		catch (const std::exception& e) {
			LOG_ERROR(name + ": must be an integer");
			return false;
		}
	}
	else {
		if (value.front() == '"' && value.back() == '"') {
			value.erase(value.begin());
			value.pop_back();
		}
		else {
			LOG_ERROR(name + ": value must have leading and trailing \"");
			return false;
		}
	}

	return true;
}
