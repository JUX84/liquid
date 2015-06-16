#pragma once

#include <unordered_map>
#include "parser.hpp"
#include "peer.hpp"

typedef std::unordered_map<std::string, Peer> PeerMap;

class Peers {
	private:
		PeerMap::iterator it;
		PeerMap pMap;
		long long lastUpdate;
	public:
		Peers();
		Peer* getPeer(const std::string&, long long);
		Peer* addPeer(const Request&, unsigned int, long long);
		void removePeer(const Request&);
		Peer* nextPeer(long long);
		unsigned long size();
		void timedOut(long long);
};
