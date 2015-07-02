#pragma once

#include <unordered_map>
#include "handler/peer.hpp"
#include "misc/parser.hpp"

typedef std::unordered_map<std::string, Peer> PeerMap;
class Database;

class Peers {
	private:
		PeerMap::iterator it;
		PeerMap pMap;
	public:
		Peers();
		Peer* getPeer(const std::string&, long long);
		Peer* addPeer(const Request&, unsigned int, bool, long long);
		void removePeer(const Request&);
		Peer* nextPeer();
		unsigned long size();
		unsigned int timedOut(long long, Database*);
};
