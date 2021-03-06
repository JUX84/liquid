#include <chrono>
#include "handler/peers.hpp"
#include "handler/requestHandler.hpp"
#include "misc/config.hpp"
#include "misc/stats.hpp"
#include "misc/utility.hpp"

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

Peer* Peers::addPeer(const Request& req, unsigned int torrentID, bool ipv6, long long now) {
	User* u = nullptr;
	if (Config::get("type") == "private")
		u = RequestHandler::getUser(req.at("reqpasskey"));
	std::string peerID = req.at("peer_id");
	pMap.emplace(peerID, Peer(req.at("ip"), req.at("port"), u, std::stoul(req.at("left")), std::stoul(req.at("downloaded")), std::stoul(req.at("uploaded")), torrentID, req.at("user-agent"), req.at("peer_id"), ipv6));
	Stats::incPeers();
	return &pMap.at(peerID);
}

void Peers::removePeer(const Request& req) {
	pMap.erase(req.at("peer_id"));
	Stats::decPeers();
}

Peer* Peers::nextPeer() {
	while (pMap.size() > 0) {
		if (it == std::end(pMap)) {
			it = std::begin(pMap);
		} else {
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
			if (Config::get("type") != "public") {
				it->second.inactive();
				db->recordPeer(&it->second);
			}
			pMap.erase(it++);
			++changed;
		} else {
			++it;
		}
	}
	return changed;
}
