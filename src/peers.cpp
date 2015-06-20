#include <chrono>
#include <iostream>
#include "peers.hpp"
#include "config.hpp"
#include "requestHandler.hpp"
#include "utility.hpp"

Peers::Peers() {
	pMap = PeerMap();
	it = std::begin(pMap);
}

Peer* Peers::getPeer(const std::string& peerID, long long now) {
	try {
		return &pMap.at(peerID);
	} catch (const std::exception& e) {
		return nullptr;
	}
}

Peer* Peers::addPeer(const Request& req, unsigned int torrentID, long long now) {
	User* u = nullptr;
	if (Config::get("type") == "private")
		u = RequestHandler::getUser(req.at("passkey"));
	std::string peerID = req.at("peer_id");
	bool seeding = req.at("left") == "0";
	pMap.emplace(peerID, Peer(req.at("ip"), req.at("port"), u, seeding, std::stoul(req.at("left")), std::stoul(req.at("downloaded")), std::stoul(req.at("uploaded")), torrentID, req.at("user-agent"), req.at("peer_id")));
	return &pMap.at(peerID);
}

void Peers::removePeer(const Request& req) {
	pMap.erase(req.at("peer_id"));
}

Peer* Peers::nextPeer(long long now) {
	while (pMap.size() > 0) {
		if (it == std::end(pMap)) {
			it = std::begin(pMap);
		} else if (!it->second.timedOut(now)) {
			PeerMap::iterator tmp = it;
			it = std::next(it);
			return &tmp->second;
		}
	}
	return nullptr;
}

unsigned long Peers::size () {
	return pMap.size();
}

unsigned int Peers::timedOut(long long now, Database* db)
{
	auto it = pMap.begin();
	unsigned int changed = 0;
	while (it != pMap.end()) {
		if (it->second.timedOut(now)) {
			if (Config::get("type") == "private") {
				it->second.inactive();
				db->recordPeer(&it->second);
				++changed;
			}
			pMap.erase(it++);
		} else {
			++it;
		}
	}
	return changed;
}
