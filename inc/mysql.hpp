#pragma once

#include <mysql.h>
#include "db.hpp"

class MySQL : public Database {
	private:
		MYSQL* mysql;
		MYSQL_RES *result;
		MYSQL_ROW row;
	public:
		void connect();
		void disconnect();
		void loadUsers(UserMap&);
		void loadTorrents(TorrentMap&);
		void loadBannedIPs(std::unordered_set<std::string>&);
		void loadLeechStatus(LeechStatus&);
		void flush();
		void flushUsers();
		void flushTorrents();
		void flushPeers();
		void flushTokens();
		void flushSnatches();
		void recordUser(User*);
		void recordTorrent(Torrent*);
		void recordPeer(Peer*);
		void recordSnatch(Peer*, long long);
		void recordToken(unsigned int, unsigned int, unsigned int, bool);
};
