#include "peermap.hpp"

PeerMap::PeerMap () {
	pMap = peerMap();
	it = std::begin(pMap);
}

User* PeerMap::getPeer(const std::string& identifier) {
	try {
		return pMap.at(identifier);
	} catch (const std::exception& e) {
		return nullptr;
	}
}

void PeerMap::addPeer(const std::string& peerID, std::string hexIP) {
	User* u = new User();
	u->setHexIP(hexIP);
	pMap.emplace(peerID, u);
}

User* PeerMap::nextPeer() {
	if (it == std::end(pMap))
		it = std::begin(pMap);
	peerMap::iterator tmp = it;
	it = std::next(it);
	return tmp->second;
}

unsigned long PeerMap::size () {
	return pMap.size();
}
