#pragma once

#include <unordered_map>
#include "parser.hpp"
#include "user.hpp"

typedef std::unordered_map<std::string, User*> PeerMap;

class Peers {
	private:
		PeerMap::iterator it;
		PeerMap pMap;
	public:
		Peers();
		User* getPeer(const std::string&);
		void addPeer(const Request&);
		void removePeer(const Request&);
		User* nextPeer();
		unsigned long size();
};
