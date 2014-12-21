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

Peer* Peers::getPeer(const std::string& peerID, const long long& now) {
	lastUpdate = now;
	try {
		return &pMap.at(peerID);
	} catch (const std::exception& e) {
		return nullptr;
	}
}

void Peers::addPeer(const Request& req, unsigned int fid, const long long& now) {
	User* u = nullptr;
	if (Config::get("type") == "private")
		u = RequestHandler::getUser(req.at("passkey"));
	pMap.emplace(req.at("peer_id"), Peer(req.at("ip"), u, req.at("left") == "0", fid, req.at("user-agent"), req.at("peer_id")));
	lastUpdate = now;
}

void Peers::removePeer(const Request& req) {
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

void Peers::timedOut(const long long& now)
{
	auto it = pMap.begin();
	while (it != pMap.end()) {
		if (it->second.timedOut(now))
			pMap.erase(it++);
		else
			++it;
	}
}
