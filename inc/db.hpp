#pragma once

#include <map>
#include <list>
#include <unordered_set>
#include "user.hpp"
#include "torrent.hpp"

typedef std::map<std::string, User*> UserMap;
typedef std::map<std::string, Torrent> TorrentMap;

enum LeechStatus {
	NORMAL,
	FREELEECH
};

class Database {
	protected:
		std::list<std::string> userRequests;
		std::list<std::string> torrentRequests;
		std::list<std::string> peerRequests;
		std::list<std::string> tokenRequests;
		std::list<std::string> snatchRequests;
	public:
		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual void loadUsers(UserMap&) = 0;
		virtual void loadTorrents(TorrentMap&) = 0;
		virtual void loadBannedIPs(std::unordered_set<std::string>&) = 0;
		virtual void loadLeechStatus(LeechStatus&) = 0;
		virtual void flush() = 0;
		virtual void flushUsers() = 0;
		virtual void flushTorrents() = 0;
		virtual void flushPeers() = 0;
		virtual void flushSnatches() = 0;
		virtual void flushTokens() = 0;
		virtual void recordUser(User*) = 0;
		virtual void recordTorrent(Torrent*) = 0;
		virtual void recordPeer(Peer*, unsigned int, long long) = 0;
		virtual void recordSnatch(Peer*, long long) = 0;
		virtual void recordToken(unsigned int, unsigned int, unsigned int, bool) = 0;
};
