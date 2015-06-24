#pragma once

#include <map>
#include <list>
#include <unordered_set>
#include <mutex>
#include "handler/torrent.hpp"
#include "handler/user.hpp"

typedef std::map<std::string, User*> UserMap;
typedef std::map<std::string, Torrent> TorrentMap;

enum LeechStatus {
	NORMAL,
	FREELEECH
};

class Database {
	protected:
		std::mutex sqlLock;
		std::list<std::string> userRequests, torrentRequests, peerRequests, tokenRequests, snatchRequests;
		std::list<std::string> userRecords, torrentRecords, peerRecords, tokenRecords, snatchRecords;
		//std::mutex userReqLock, torrentReqLock, peerReqLock, tokenReqLock, snatchReqLock;
		bool usersFlushing, torrentsFlushing, peersFlushing, tokensFlushing, snatchesFlushing;
	public:
		virtual void connect() = 0;
		virtual void disconnect() = 0;
		virtual void reset() = 0;
		virtual void loadUsers(UserMap&) = 0;
		virtual void loadTorrents(TorrentMap&) = 0;
		virtual void loadBannedIPs(std::unordered_set<std::string>&) = 0;
		virtual void loadClientWhitelist(std::list<std::string>&) = 0;
		virtual void loadLeechStatus(LeechStatus&) = 0;
		virtual void flush() = 0;
		virtual void flushUsers() = 0;
		virtual void flushTorrents() = 0;
		virtual void flushPeers() = 0;
		virtual void flushSnatches() = 0;
		virtual void flushTokens() = 0;
		virtual void doFlushUsers() = 0;
		virtual void doFlushTorrents() = 0;
		virtual void doFlushPeers() = 0;
		virtual void doFlushSnatches() = 0;
		virtual void doFlushTokens() = 0;
		virtual void recordUser(User*) = 0;
		virtual void recordTorrent(Torrent*) = 0;
		virtual void recordPeer(Peer*) = 0;
		virtual void recordSnatch(Peer*, long long) = 0;
		virtual void recordToken(unsigned int, unsigned int, unsigned int, bool) = 0;
};
