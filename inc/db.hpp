#pragma once

#include <map>
#include <list>
#include <forward_list>
#include "user.hpp"
#include "torrent.hpp"

typedef std::map<std::string, User*> UserMap;
typedef std::map<std::string, Torrent> TorrentMap;

class Database {
	protected:
		std::list<std::string> requests;
	public:
		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual void loadUsers(UserMap&) = 0;
		virtual void loadTorrents(TorrentMap&) = 0;
		virtual void loadBannedIPs(std::forward_list<std::string>&) = 0;
		virtual void record(std::string) = 0;
		virtual void flush() = 0;
		virtual void recordUser(User*) = 0;
		virtual void recordPeer(Peer*, unsigned int, long long) = 0;
		virtual void recordPeerSnatch(Peer*, long long) = 0;
		virtual void recordPeerRemoval(Peer*) = 0;
};
