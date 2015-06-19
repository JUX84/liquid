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
		static void changePasskey(const Request*);
		static void addTorrent(const Request*, const std::string&);
		static void deleteTorrent(const std::string&);
		static void updateTorrent(const Request*, const std::string&);
		static void addUser(const Request*);
		static void updateUser(const Request*);
		static void removeUser(const Request*);
		static void removeUsers(const Request*);
		static void addBan(const Request*);
		static void removeBan(const Request*);
		static void addWhitelist(const Request*);
		static void editWhitelist(const Request*);
		static void removeWhitelist(const Request*);
		static void addIPRestriction(const Request*);
		static void removeIPRestriction(const Request*);
		static void addToken(const Request*, const std::string&);
		static void removeToken(const Request*, const std::string&);
		static void setLeechStatus(const Request*);
	public:
		static void init();
		static std::string handle(std::string, std::string);
		static User* getUser(const std::string&);
		static void stop();
		static void clearTorrentPeers(ev::timer&, int);
		static void flushSqlRecords(ev::timer&, int);
};
