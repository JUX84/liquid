#include <algorithm>
#include <stdexcept>
#include <chrono>
#include "database/mysql.hpp"
#include "handler/requestHandler.hpp"
#include "handler/torrent.hpp"
#include "misc/config.hpp"
#include "misc/logger.hpp"
#include "misc/response.hpp"
#include "misc/stats.hpp"
#include "misc/utility.hpp"

TorrentMap RequestHandler::torMap;
UserMap RequestHandler::usrMap;
Database* RequestHandler::db;
std::unordered_set<std::string> RequestHandler::bannedIPs;
std::list<std::string> RequestHandler::clientWhitelist;
LeechStatus RequestHandler::leechStatus;

std::string RequestHandler::handle(std::string str, std::string ip, bool ipv6)
{
	LOG_INFO("Handling new request");
	std::pair<Request, std::forward_list<std::string>> infos = Parser::parse(str); // parse the request
	Request* req = &infos.first;
	std::forward_list<std::string>* infoHashes = &infos.second;
	std::string check = Parser::check(*req); // check if we have all we need to process (saves time if not the case
	if (check != "success") { // missing params
		LOG_WARNING("Couldn't parse request (" + check + ")");
		return error(check);
	}
	if (Config::get("type") == "private" && req->at("action") == "update" && req->at("reqpasskey") == Config::get("updatekey"))
		return update(req, infoHashes);
	User* u = getUser(req->at("reqpasskey"));
	if (Config::get("type") == "private" && u == nullptr) {
		LOG_WARNING("Passkey " + req->at("reqpasskey") + " not found");
		return error("passkey not found");
	}
	if (infoHashes->begin() == infoHashes->end()) {
		LOG_WARNING("Missing info hash");
		return error("missing info_hash");
	}
	if (req->find("ip") == req->end())
		req->emplace("ip", ip);
	if (bannedIPs.find(req->at("ip")) != bannedIPs.end())
		return error("banned ip");
	if (Config::get("type") == "private" && u->isRestricted(req->at("ip")))
		return error("ip not associated with account");
	if (req->at("action") == "announce") {
		if (req->at("peer_id").length() != 20)
			req->at("peer_id") = Utility::hex_to_bin(req->at("peer_id"));
		if (req->at("peer_id").length() != 20)
			return error("invalid peer id");
		req->emplace("event", "updating");
		return announce(req, infoHashes->front(), ipv6);
	}
	else if (req->at("action") == "scrape")
		return scrape(infoHashes);
	LOG_ERROR("Unexpected! Action not found");
	return error("invalid action"); // not possible, since the request is checked, but, well, who knows :3
}

std::string RequestHandler::announce(const Request* req, const std::string& infoHash, bool ipv6)
{
	LOG_INFO("Announce request (" + req->at("event") + ")");
	if (clientWhitelist.size() > 0) {
		bool whitelisted = false;
		for (const auto &it : clientWhitelist) {
			if (req->at("peer_id").compare(0, it.length(), it) == 0) {
				whitelisted = true;
				break;
			}
		}
		if (!whitelisted)
			return error("client not in whitelist");
	}
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	if (Config::get("type") == "public" && torMap.find(infoHash) == torMap.end()) {
		torMap.emplace(infoHash, Torrent(Stats::getTorrents(), 0, 0, 0));
		Stats::incTorrents();
	}
	Torrent *tor = nullptr;
	try {
		tor = &torMap.at(infoHash);
	} catch (const std::exception& e) {
		LOG_WARNING("Torrent not found");
		return error("torrent not found");
	}
	if (Config::get("type") == "private" && !getUser(req->at("reqpasskey"))->isAuthorized())
		return error("user unauthorized");
	Peers *peers = nullptr;
	Peers *peers6 = nullptr;
	Peer *peer = nullptr;
	long balance = 0;
	int free = 0;
	unsigned long corrupt;
	try {
		corrupt = std::stoul(req->at("corrupt"));
	} catch (const std::exception& e) {
		corrupt = 0;
	}
	bool compact = true;
	if (req->find("compact") != req->end() && req->at("compact") == "0")
		compact = false;
	if (req->at("left") != "0" || req->at("event") == "completed") {
		if (ipv6)
			peer = tor->getLeechers6()->getPeer(req->at("peer_id"), now);
		else
			peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
		if (peer == nullptr) {
			if (ipv6)
				peer = tor->getLeechers6()->addPeer(*req, tor->getID(), ipv6, now);
			else
				peer = tor->getLeechers()->addPeer(*req, tor->getID(), ipv6, now);
		}
		if (Config::get("type") == "private") {
			free = tor->getFree();
			if (leechStatus == FREELEECH)
				free = 100;
			if (peer->getUser()->hasToken(tor->getID())) {
				bool expired = peer->getUser()->isTokenExpired(tor->getID());
				db->recordToken(peer->getUser()->getID(), tor->getID(), peer->getTotalDownloaded()-std::stoul(req->at("downloaded")), expired);
				free = 100;
			}
		}
		if (req->at("event") == "completed") {
			peer->complete();
			tor->incSnatches();
		}
		if (ipv6)
			peers6 = tor->getSeeders6();
		peers = tor->getSeeders();
	}
	if (req->at("left") == "0") {
		if (ipv6)
			peer = tor->getSeeders6()->getPeer(req->at("peer_id"), now);
		else
			peer = tor->getSeeders()->getPeer(req->at("peer_id"), now);
		if (peer == nullptr) {
			if (ipv6)
				peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
			else
				peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
			if (peer == nullptr) {
				if (ipv6)
					peer = tor->getSeeders6()->addPeer(*req, tor->getID(), ipv6, now);
				else
					peer = tor->getSeeders()->addPeer(*req, tor->getID(), ipv6, now);
			}
		}
		if (ipv6)
			peers6 = tor->getLeechers6();
		peers = tor->getLeechers();
	}
	if (req->at("event") == "stopped")
		peer->inactive();
	if (Config::get("type") == "private") {
		balance -= peer->getCorrupt()-corrupt;
		peer->updateStats(std::stoul(req->at("downloaded"))*(1-(free/100)), std::stoul(req->at("uploaded")), std::stoul(req->at("left")), corrupt, now);
		balance -= peer->getDownloaded();
		balance += peer->getUploaded();
	}
	tor->setBalance(balance);
	if (Config::get("type") != "public")
		db->recordPeer(peer);
	if (Config::get("type") == "private") {
		peer->getUser()->updateStats(peer->getDownloaded(),peer->getUploaded());
		db->recordUser(peer->getUser());
	}
	if (req->at("event") == "stopped") {
		if (req->at("left") != "0") {
			if (ipv6)
				tor->getLeechers6()->removePeer(*req);
			else
				tor->getLeechers()->removePeer(*req);
		} else {
			if (ipv6)
				tor->getSeeders6()->removePeer(*req);
			else
				tor->getSeeders()->removePeer(*req);
		}
	}
	if (req->at("event") == "completed") {
		if (ipv6)
			tor->getLeechers()->removePeer(*req);
		else
			tor->getLeechers6()->removePeer(*req);
	}
	if (Config::get("type") != "public")
		db->recordTorrent(tor);
	std::string peerlist6 = "";
	int i = 0;
	int j = 0;
	if (req->at("event") != "stopped") {
		try {
			i = std::stoi(req->at("numwant"));
		} catch (const std::exception& e) {
			i = Config::getInt("default_numwant");;
		}
		i = std::min(std::min(i, Config::getInt("max_numwant")), static_cast<int>(peers->size()));
		if (ipv6)
			j = std::min(std::min(i, Config::getInt("max_numwant")), static_cast<int>(peers6->size()));
		i -= j;
	}
	if (j > 0)
		peerlist6 = getPeers(peers6, j, true, compact);
	std::string peerlist = getPeers(peers, i, false, compact);
	bool gzip = false;
	try {
		if (req->at("accept-encoding").find("gzip") != std::string::npos && (i*6 + j*18) > 100)
			gzip = true;
	} catch (const std::exception& e) {}
	return response(
			("d8:completei"
			+ std::to_string(tor->getSeeders()->size() + tor->getSeeders6()->size())
			+ "e10:incompletei"
			+ std::to_string(tor->getLeechers()->size() + tor->getLeechers6()->size())
			+ "e10:downloadedi"
			+ std::to_string(tor->getSnatches())
			+ "e8:intervali"
			+ Config::get("default_announce_interval")
			+ "e12:min intervali"
			+ Config::get("min_announce_interval")
			+ "e"
			+ peerlist6
			+ peerlist
			+ "e"),
			gzip);
}

std::string RequestHandler::getPeers(Peers* peers, int numwant, bool ipv6, bool compact) {
	std::string peerlist;
	if (ipv6)
		peerlist.append("6:peers6");
	else
		peerlist.append("5:peers");
	int bytes = 6;
	if (ipv6)
		bytes = 18;
	if (compact)
		peerlist.append(std::to_string(numwant*bytes) + ":");
	else
		peerlist.append("d");
	while (numwant-- > 0) {
		Peer* p = peers->nextPeer();
		if (p != nullptr) {
			if (compact) {
				peerlist.append(p->getHexIPPort());
			} else {
				peerlist.append("7:peer id" + std::to_string(p->getPeerID().length()) + ":" + p->getPeerID() +
						"2:ip" + std::to_string(p->getIP().length()) + ":" + p->getIP() +
						"4:port" + std::to_string(p->getPort().length()) + ":" + p->getPort());
			}
		}
	}
	if (!compact)
		peerlist.append("e");
	return peerlist;
}

std::string RequestHandler::scrape(const std::forward_list<std::string>* infoHashes)
{
	LOG_INFO("Scrape request");
	std::string resp("d5:filesd");

	for (const auto& infoHash : *infoHashes) {
		const TorrentMap::iterator it = torMap.find(infoHash);
		if (it != torMap.end()) {
			resp += std::to_string(infoHash.length())
				+ ":"
				+ infoHash
				+ "d8:completei"
				+ std::to_string(it->second.getSeeders()->size() + it->second.getSeeders6()->size())
				+ "e10:incompletei"
				+ std::to_string(it->second.getLeechers()->size() + it->second.getLeechers()->size())
				+ "e10:downloadedi"
				+ std::to_string(it->second.getSnatches())
				+ "ee";
		}
	}
	resp += "ee";

	return response(resp);
}

std::string RequestHandler::update(const Request* req, const std::forward_list<std::string>* infoHashes)
{
	const std::string& type = req->at("type");
	LOG_INFO("Update request (" + type + ")");

	if (type == "change_passkey")
		changePasskey(req);
	else if (type == "add_torrent")
		addTorrent(req, infoHashes->front());
	else if (type == "delete_torrent")
		deleteTorrent(infoHashes->front());
	else if (type == "update_torrent")
		updateTorrent(req, infoHashes->front());
	else if (type == "add_user")
		addUser(req);
	else if (type == "update_user")
		updateUser(req);
	else if (type == "remove_user")
		removeUser(req);
	else if (type == "remove_users")
		removeUsers(req);
	else if (type == "add_token")
		addToken(req, infoHashes->front());
	else if (type == "remove_token")
		removeToken(req, infoHashes->front());
	else if (type == "add_ban")
		addBan(req);
	else if (type == "remove_ban")
		removeBan(req);
	else if (type == "add_ip_restriction")
		addIPRestriction(req);
	else if (type == "remove_ip_restriction")
		removeIPRestriction(req);
	else if (type == "add_whitelist")
		addWhitelist(req);
	else if (type == "edit_whitelist")
		editWhitelist(req);
	else if (type == "remove_whitelist")
		removeWhitelist(req);
	else if (type == "set_leech_status")
		setLeechStatus(req);

	return response("success");
}

void RequestHandler::addIPRestriction(const Request* req)
{
	std::string ip = Utility::long2ip(std::stoul(req->at("ip")));
	const std::string& passkey = req->at("passkey");
	usrMap.at(passkey)->addIPRestriction(ip);
	LOG_INFO("Added IP Restriction " + ip + " for User " + std::to_string(getUser(passkey)->getID()));
}

void RequestHandler::removeIPRestriction(const Request* req)
{
	std::string ip = Utility::long2ip(std::stoul(req->at("ip")));
	const std::string& passkey = req->at("passkey");
	usrMap.at(passkey)->removeIPRestriction(ip);
	LOG_INFO("Removed IP Restriction " + ip + " for User " + std::to_string(getUser(passkey)->getID()));
}

void RequestHandler::addToken(const Request* req, const std::string& infoHash)
{
	unsigned int torrentID = torMap.at(infoHash).getID();
	const std::string& passkey = req->at("passkey");
	auto duration = std::chrono::system_clock::now().time_since_epoch();
    long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	usrMap.at(passkey)->addToken(torrentID, now);
	LOG_INFO("Added Token for User " + std::to_string(getUser(passkey)->getID()) + " on Torrent " + std::to_string(torrentID));
}

void RequestHandler::removeToken(const Request* req, const std::string& infoHash)
{
	unsigned int torrentID = torMap.at(infoHash).getID();
	const std::string& passkey = req->at("passkey");
	usrMap.at(passkey)->removeToken(torrentID);
	LOG_INFO("Removed Token for User " + std::to_string(getUser(passkey)->getID()) + " on Torrent " + std::to_string(torrentID));
}

void RequestHandler::changePasskey(const Request* req)
{
	const std::string& oldPasskey = req->at("old_passkey");
	const std::string& newPasskey = req->at("new_passkey");
	User* u = getUser(oldPasskey);
	if (u != nullptr) {
		usrMap.erase(oldPasskey);
		usrMap.emplace(newPasskey, u);
		LOG_INFO("Changed passkey for User " + std::to_string(u->getID()) + "(" + oldPasskey + " -> " + newPasskey + ")");
	} else {
		LOG_INFO("Passkey " + oldPasskey + " not found");
	}
}

void RequestHandler::addTorrent(const Request* req, const std::string& infoHash)
{
	const std::string& torrentID = req->at("id");
	auto t = torMap.emplace(infoHash, Torrent(std::stoul(torrentID), 0, 0, 0));
	if (req->at("freetorrent") == "1")
		t.first->second.setFree(100);
	else if (req->at("freetorrent") == "2")
		t.first->second.setFree(50);
	if (t.second)
		LOG_INFO("Added Torrent " + torrentID);
	else
		LOG_INFO("Torrent " + torrentID + " already exists");
}

void RequestHandler::deleteTorrent(const std::string& infoHash)
{
	const auto it = torMap.find(infoHash);
	if (it != torMap.end()) {
		torMap.erase(it);
		LOG_INFO("Deleted Torrent " + std::to_string(it->second.getID()));
	}
}

void RequestHandler::updateTorrent(const Request* req, const std::string& infoHash)
{
	auto it = torMap.find(infoHash);
	if (it != torMap.end()) {
		if (req->at("freetorrent") == "1")
			it->second.setFree(100);
		else if (req->at("freetorrent") == "2")
			it->second.setFree(50);
		else
			it->second.setFree(0);
		LOG_INFO("Updated Torrent " + std::to_string(it->second.getID()));
	}
}

void RequestHandler::addUser(const Request* req)
{
	std::string passkey = req->at("passkey");
	std::string userID = req->at("id");
	if (usrMap.emplace(passkey, new User(std::stoul(userID), true)).second)
		LOG_INFO("Added User " + userID + " (" + passkey + ")");
	else
		LOG_INFO("User " + userID + " (" + passkey + ") already exists");
}

void RequestHandler::updateUser(const Request* req) {
	std::string passkey = req->at("passkey");
	bool authorized = req->at("can_leech") == "1";
	User* u = getUser(passkey);
	if (u != nullptr) {
		getUser(passkey)->setAuthorized(authorized);
		LOG_INFO("User " + std::to_string(u->getID()) + " " + (authorized ? "authorized" : "unauthorized"));
	}
}

void RequestHandler::removeUser(const Request* req)
{
	std::string passkey = req->at("passkey");
	User* u = getUser(passkey);
	if (u != nullptr) {
		usrMap.erase(passkey);
		LOG_INFO("Removed User " + std::to_string(u->getID()) + " (" + passkey + ")");
	}
}

void RequestHandler::removeUsers(const Request* req)
{
	std::string passkeys = req->at("passkeys");
	for (unsigned int i = 0; i < passkeys.length(); i += 32) {
		std::string passkey = passkeys.substr(i, 32);
		User* u = getUser(passkey);
		if (u != nullptr) {
			usrMap.erase(passkey);
			LOG_INFO("Removed User " + std::to_string(u->getID()) + " (" + passkey + ")");
		}
	}
}

void RequestHandler::addBan(const Request* req) {
	unsigned int from = std::stoul(req->at("fromip"));
	unsigned int to = std::stoul(req->at("toip"));
	while (from != to)
		bannedIPs.emplace(Utility::long2ip(from++));
	bannedIPs.emplace(Utility::long2ip(from));
	LOG_INFO("Added ban IP range (" + std::to_string(from) + " - " + std::to_string(to) + ")");
}

void RequestHandler::removeBan(const Request* req) {
	unsigned int from = std::stoul(req->at("fromip"));
	unsigned int to = std::stoul(req->at("toip"));
	while (from != to)
		bannedIPs.erase(Utility::long2ip(from++));
	bannedIPs.erase(Utility::long2ip(from));
	LOG_INFO("Removed ban IP range (" + std::to_string(from) + " - " + std::to_string(to) + ")");
}

void RequestHandler::addWhitelist(const Request* req) {
	std::string peerID = req->at("peer_id");
	clientWhitelist.push_back(peerID);
	LOG_INFO("Added client whitelist (" + peerID + ")");
}

void RequestHandler::editWhitelist(const Request* req) {
	std::string oldPeerID = req->at("old_peer_id");
	std::string newPeerID = req->at("new_peer_id");
	clientWhitelist.remove(oldPeerID);
	clientWhitelist.push_back(newPeerID);
	LOG_INFO("Edited client whitelist (" + oldPeerID + " -> " + newPeerID + ")");
}

void RequestHandler::removeWhitelist(const Request* req) {
	std::string peerID = req->at("peer_id");
	clientWhitelist.remove(peerID);
	LOG_INFO("Removed client whitelist (" + peerID + ")");
}

void RequestHandler::setLeechStatus(const Request* req) {
	leechStatus = (req->at("leech_status") == "freeleech" ? FREELEECH : NORMAL);
	std::string LeechStatus = (leechStatus == FREELEECH ? "FREE" : "NORMAL");
	LOG_INFO("Updated leech status (" + LeechStatus  + ")");
}

User* RequestHandler::getUser(const std::string& passkey) {
	try {
		return usrMap.at(passkey);
	} catch (const std::exception& e) {
		return nullptr;
	}
}

void RequestHandler::init() {
	LOG_INFO("Initializing request handler");
	if (Config::get("type") != "public") {
		db = new MySQL();
		db->connect();
		db->loadTorrents(torMap);
		db->loadBannedIPs(bannedIPs);
		db->loadClientWhitelist(clientWhitelist);
		if (Config::get("type") == "private") {
			db->loadUsers(usrMap);
			db->loadLeechStatus(leechStatus);
		}
	}
}

void RequestHandler::stop() {
	LOG_INFO("Stopping request handler");
	if (Config::get("type") != "public") {
		db->flush();
		db->disconnect();
	}
}

void RequestHandler::clearTorrentPeers(ev::timer& timer, int revents)
{
	LOG_INFO("Checking timed out peers");
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	unsigned int changed;
	unsigned int changedTotal = 0;
	auto t = torMap.begin();
	while (t != torMap.end()) {
		changed = t->second.getSeeders()->timedOut(now, db);
		changed += t->second.getSeeders6()->timedOut(now, db);
		changed += t->second.getLeechers()->timedOut(now, db);
		changed += t->second.getLeechers6()->timedOut(now, db);
		if(Config::get("type") == "public") {
			if (t->second.getSeeders()->size() == 0 && t->second.getSeeders6()->size() == 0 && t->second.getLeechers()->size() == 0 && t->second.getLeechers6()->size() == 0) {
				torMap.erase(t++);
				Stats::decTorrents();
			} else {
				++t;
			}
		} else {
			if (changed > 0)
				db->recordTorrent(&t->second);
			++t;
		}
		changedTotal += changed;
	}
	LOG_INFO(std::to_string(changedTotal) + " new inactive peers");
}

void RequestHandler::flushSqlRecords(ev::timer& timer, int revents) {
	if (Config::get("type") != "public")
		db->flush();
}
