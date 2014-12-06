#include "peers.hpp"
#include "config.hpp"
#include "requestHandler.hpp"

Peers::Peers () {
	pMap = PeerMap();
	it = std::begin(pMap);
}

Peer* Peers::getPeer(const std::string& peerID) {
	try {
		return &pMap.at(peerID);
	} catch (const std::exception& e) {
		return nullptr;
	}
}

void Peers::addPeer(const Request& req, unsigned int fid) {
	User* u = nullptr;
	if (Config::get("type") == "private")
		u = RequestHandler::getUser(req.at("passkey"));
	pMap.emplace(req.at("peer_id"), Peer(req.at("ip"), u, req.at("left") == "0", fid, req.at("user-agent")));
}

void Peers::removePeer(const Request& req) {
	// TODO clean users peers
	pMap.erase(req.at("peer_id"));
}

Peer* Peers::nextPeer(const long long &now) {
	while (pMap.size() > 0) {
		if (it == std::end(pMap)) {
			it = std::begin(pMap);
			continue;
		} else if (it->second.timedOut(now)) {
			pMap.erase(it);
			continue;
		}
		PeerMap::iterator tmp = it;
		it = std::next(it);
		return &tmp->second;
	}
	return nullptr;
}

unsigned long Peers::size () {
	return pMap.size();
}
