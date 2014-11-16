#include "peermap.hpp"
#include "utility.hpp"

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

void PeerMap::addPeer(const request& req) {
	User* u;
	//if (Config::get("type") == "private")
	//	u = UserMap::getUser(req.at("passkey"));
	//else
		u = new User();
	if (pMap.find(req.at("peer_id")) == pMap.end()) {
		u->addHexIP(       
			(Utility::ip_hex_encode(req.at("ip"))
		 	+
		 	Utility::port_hex_encode(req.at("port")))
			);
	}
	pMap.emplace(req.at("peer_id"), u);
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
