#pragma once

#include <ev++.h>
#include "db.hpp"
#include "parser.hpp"

class RequestHandler {
	private:
		static TorrentMap torMap;
		static UserMap usrMap;
		static Database *db;
		static std::unordered_set<std::string> bannedIPs;
		static std::list<std::string> clientWhitelist;
		static LeechStatus leechStatus;
		static std::string announce(const Request*, const std::string&, bool);
		static std::string scrape(const std::forward_list<std::string>*, bool);
		static std::string update(const Request*, const std::forward_list<std::string>*);
		static std::string changePasskey(const Request*);
		static std::string addTorrent(const Request*, const std::string&);
		static std::string deleteTorrent(const std::string&);
		static std::string updateTorrent(const Request*, const std::string&);
		static std::string addUser(const Request*);
		static std::string updateUser(const Request*);
		static std::string removeUser(const Request*);
		static std::string addBan(const Request*);
		static std::string removeBan(const Request*);
		static std::string addWhitelist(const Request*);
		static std::string removeWhitelist(const Request*);
		static std::string addIPRestriction(const Request*);
		static std::string removeIPRestriction(const Request*);
		static std::string addToken(const Request*, const std::string&);
		static std::string removeToken(const Request*, const std::string&);
		static std::string setLeechStatus(const Request*);
	public:
		static void init();
		static std::string handle(std::string, std::string);
		static User* getUser(const std::string&);
		static void stop();
		static void clearTorrentPeers(ev::timer&, int);
		static void flushSqlRecords(ev::timer&, int);
};
