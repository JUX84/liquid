#pragma once

#include <unordered_map>
#include "parser.hpp"
#include "user.hpp"

typedef std::unordered_map<std::string, User*> peerMap;

class PeerMap {
	private:
		peerMap::iterator it;
		peerMap pMap;
	public:
		PeerMap();
		User* getPeer(const std::string&);
		void addPeer(const request&);
		void removePeer(const request&);
		User* nextPeer();
		unsigned long size();
};
