#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include "config.hpp"
#include "logger.hpp"
#include "requestHandler.hpp"
#include "response.hpp"
#include "torrent.hpp"
#include "utility.hpp"
#include "mysql.hpp"

TorrentMap RequestHandler::torMap;
UserMap RequestHandler::usrMap;
Database* RequestHandler::db;
std::unordered_set<std::string> RequestHandler::bannedIPs;
std::list<std::string> RequestHandler::clientWhitelist;
LeechStatus RequestHandler::leechStatus;

std::string RequestHandler::handle(std::string str, std::string ip, bool ipv6)
{
	LOG_INFO("Handling new request");
	if (ipv6) {
		LOG_ERROR("IPv6 unsupported: ip = " + ip);
		return error("IPv6 unsupported");
	}
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
	if (u->isRestricted(req->at("ip")))
		return error("ip not associated with account");
	if (req->at("action") == "announce") {
		if (req->find("compact") != req->end() && req->at("compact") == "0")
			return error("client does not support compact");
		req->emplace("event", "updating");
		return announce(req, infoHashes->front());
	}
	else if (req->at("action") == "scrape")
		return scrape(infoHashes);
	LOG_ERROR("Unexpected! Action not found");
	return error("invalid action"); // not possible, since the request is checked, but, well, who knows :3
}

std::string RequestHandler::announce(const Request* req, const std::string& infoHash)
{
	LOG_INFO("Announce request");
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
	if (Config::get("type") != "private")
		torMap.emplace(infoHash, Torrent(0, 0, 0));
	Torrent *tor = nullptr;
	try {
		tor = &torMap.at(infoHash);
	} catch (const std::exception& e) {
		LOG_WARNING("Torrent not found");
		return error("torrent not found");
	}
	if (!getUser(req->at("reqpasskey"))->isAuthorized())
		return error("user unauthorized");
	Peers *peers = nullptr;
	Peer *peer = nullptr;
	long balance = 0;
	int free = 0;
	unsigned long corrupt;
	try {
		corrupt = std::stoul(req->at("corrupt"));
	} catch (const std::exception& e) {
		corrupt = 0;
	}
	if (req->at("left") != "0" || req->at("event") == "completed") {
		peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
		if (peer == nullptr) {
			peer = tor->getLeechers()->addPeer(*req, tor->getID(), now);
			tor->change();
		} else {
			peer->complete();
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
		peers = tor->getSeeders();
	}
	if (req->at("left") == "0") {
		peer = tor->getSeeders()->getPeer(req->at("peer_id"), now);
		if (peer == nullptr) {
			peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
			if (peer == nullptr) {
				peer = tor->getSeeders()->addPeer(*req, tor->getID(), now);
				tor->change();
			}
		}
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
	db->recordPeer(peer);
	db->recordUser(peer->getUser());
	if (req->at("event") == "stopped") {
		if (req->at("left") != "0")
			tor->getLeechers()->removePeer(*req);
		else
			tor->getSeeders()->removePeer(*req);
		tor->change();
	}
	if (req->at("event") == "completed") {
		tor->getLeechers()->removePeer(*req);
		tor->change();
	}
	db->recordTorrent(tor);
	std::string peerlist;
	unsigned long i = 0;
	if (req->at("event") != "stopped") {
		try {
			i = std::stoi(req->at("numwant"));
		} catch (const std::exception& e) {
			i = Config::getInt("default_numwant");;
		}
		i = std::min(std::min(i, static_cast<unsigned long>(Config::getInt("max_numwant"))), peers->size());
	}
	while (i-- > 0) {
		Peer* p = peers->nextPeer(now);
		if (p != nullptr)
			peerlist.append(p->getHexIPPort());
	}
	return response(
			("d8:completei"
			+ std::to_string(tor->getSeeders()->size())
			+ "e10:incompletei"
			+ std::to_string(tor->getLeechers()->size())
			+ "e10:downloadedi"
			+ std::to_string(tor->getSnatches())
			+ "e8:intervali"
			+ std::to_string(900)
			+ "e12:min intervali"
			+ std::to_string(300)
			+ "e5:peers"
			+ std::to_string(peerlist.length())
			+ ":"
			+ peerlist
			+ "e"));
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
				+ std::to_string(it->second.getSeeders()->size())
				+ "e10:incompletei"
				+ std::to_string(it->second.getLeechers()->size())
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
	auto t = torMap.emplace(infoHash, Torrent(std::stoul(torrentID), 0, 0));
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
		std::string leech;
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
	db = new MySQL();
	db->connect();
	db->loadUsers(usrMap);
	db->loadTorrents(torMap);
	db->loadBannedIPs(bannedIPs);
	db->loadClientWhitelist(clientWhitelist);
	if (Config::get("type") == "private")
		db->loadLeechStatus(leechStatus);
}

void RequestHandler::stop() {
	LOG_INFO("Stopping request handler");
	db->flush();
	db->disconnect();
}

void RequestHandler::clearTorrentPeers(ev::timer& timer, int revents)
{
	LOG_INFO("Checking timed out peers");
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	unsigned int changed = 0;
	auto t = torMap.begin();
	while (t != torMap.end()) {
		changed += t->second.getSeeders()->timedOut(now, db);
		changed += t->second.getLeechers()->timedOut(now, db);
		if(Config::get("type") == "public") {
			if (t->second.getSeeders()->size() == 0 && t->second.getLeechers()->size() == 0)
				torMap.erase(t++);
		} else {
			if (changed > 0) {
				LOG_INFO(std::to_string(changed) + " new inactive peers");
				db->recordTorrent(&t->second);
			}
			++t;
		}
	}
}

void RequestHandler::flushSqlRecords(ev::timer& timer, int revents) {
	db->flush();
}
