#include "peers.hpp"
#include "config.hpp"
#include "requestHandler.hpp"

Peers::Peers () {
	pMap = PeerMap();
	it = std::begin(pMap);
}

Peer* Peers::getPeer(const std::string& identifier) {
	try {
		return &pMap.at(identifier);
	} catch (const std::exception& e) {
		return nullptr;
	}
}

void Peers::addPeer(const Request& req) {
	User* u;
	if (Config::get("type") == "private")
		u = RequestHandler::getUser(req.at("passkey"));
	else
		u = new User();
	if (u->getHexIP()->find(req.at("ip")+":"+req.at("port")) == u->getHexIP()->end())
		u->addHexIP(req);
	pMap.emplace(req.at("peer_id"), Peer(u));
}

void Peers::removePeer(const Request& req) {
	pMap.erase(req.at("peer_id"));
}

Peer* Peers::nextPeer() {
	if (it == std::end(pMap))
		it = std::begin(pMap);
	PeerMap::iterator tmp = it;
	it = std::next(it);
	return &tmp->second;
}

unsigned long Peers::size () {
	return pMap.size();
}
