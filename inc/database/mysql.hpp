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
		void reset();
		void loadUsers(UserMap&);
		void loadTorrents(TorrentMap&);
		void loadBannedIPs(std::unordered_set<std::string>&);
		void loadClientWhitelist(std::list<std::string>&);
		void loadLeechStatus(LeechStatus&);
		void flush();
		void flushUsers();
		void flushTorrents();
		void flushPeers();
		void flushTokens();
		void flushSnatches();
		void doFlushUsers();
		void doFlushTorrents();
		void doFlushPeers();
		void doFlushTokens();
		void doFlushSnatches();
		void recordUser(User*);
		void recordTorrent(Torrent*);
		void recordPeer(Peer*);
		void recordSnatch(Peer*, long long);
		void recordToken(unsigned int, unsigned int, unsigned int, bool);
};
