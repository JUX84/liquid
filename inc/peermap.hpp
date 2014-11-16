#pragma once

#include <unordered_map>
#include "user.hpp"

typedef std::unordered_map<std::string, User*> peerMap;

class PeerMap {
	private:
		peerMap::iterator it;
		peerMap pMap;
	public:
		PeerMap();
		User* getPeer(const std::string&);
		void addPeer(const std::string&, std::string);
		User* nextPeer();
		unsigned long size();
};
