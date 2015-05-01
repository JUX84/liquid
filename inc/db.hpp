#pragma once

#include <map>
#include <list>
#include <unordered_set>
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
		virtual void loadBannedIPs(std::unordered_set<std::string>&) = 0;
		virtual void record(std::string) = 0;
		virtual void flush() = 0;
		virtual void recordUser(User*) = 0;
		virtual void recordTorrent(Torrent*) = 0;
		virtual void recordPeer(Peer*, unsigned int, long long) = 0;
		virtual void recordPeerSnatch(Peer*, long long) = 0;
		virtual void recordPeerRemoval(Peer*) = 0;
		virtual void recordTokenExpiration(std::string, std::string) = 0;
		virtual void recordSnatch(Torrent*) = 0;
};
