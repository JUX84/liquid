#include "peermap.hpp"
#include "config.hpp"
#include "requestHandler.hpp"

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
	if (Config::get("type") == "private")
		u = RequestHandler::getUser(req.first.at("passkey"));
	else
		u = new User();
	if (u->getHexIP()->find(req.first.at("ip")+":"+req.first.at("port")) == u->getHexIP()->end())
		u->addHexIP(req);
	pMap.emplace(req.first.at("peer_id"), u);
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
