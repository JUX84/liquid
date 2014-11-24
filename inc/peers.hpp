#pragma once

#include <unordered_map>
#include "parser.hpp"
#include "peer.hpp"

typedef std::unordered_map<std::string, Peer> PeerMap;

class Peers {
	private:
		PeerMap::iterator it;
		PeerMap pMap;
	public:
		Peers();
		Peer* getPeer(const std::string&);
		void addPeer(const Request&, unsigned int);
		void removePeer(const Request&);
		Peer* nextPeer();
		unsigned long size();
};
