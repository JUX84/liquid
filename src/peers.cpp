#include <chrono>
#include <iostream>
#include "peers.hpp"
#include "config.hpp"
#include "requestHandler.hpp"

Peers::Peers() {
	pMap = PeerMap();
	it = std::begin(pMap);
	auto time_point = std::chrono::system_clock::now();
	auto duration = time_point.time_since_epoch();
	lastUpdate = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

Peer* Peers::getPeer(const std::string& peerID, long long now) {
	lastUpdate = now;
	try {
		return &pMap.at(peerID);
	} catch (const std::exception& e) {
		return nullptr;
	}
}

Peer* Peers::addPeer(const Request& req, unsigned int fid, long long now) {
	User* u = nullptr;
	if (Config::get("type") == "private")
		u = RequestHandler::getUser(req.at("passkey"));
	Peer* p = new Peer(req.at("ip"), u, req.at("left") == "0", std::stoul(req.at("left")), fid, req.at("user-agent"), req.at("peer_id"));
	pMap.emplace(req.at("peer_id"), *p);
	lastUpdate = now;
	return p;
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
				db->recordPeer(&it->second, now);
				++changed;
			}
			pMap.erase(it++);
		}
	}
	return changed;
}
