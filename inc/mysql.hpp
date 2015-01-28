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
		void loadBannedIPs(std::forward_list<std::string>&);
		void record(std::string);
		void flush();
		void recordUser(User*);
		void recordPeer(Peer*, unsigned int, long long);
		void recordPeerSnatch(Peer*, long long);
		void recordPeerRemoval(Peer*);
		void recordSnatch(Torrent*);
};
