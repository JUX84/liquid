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

std::string RequestHandler::handle(std::string str, std::string ip)
{
	LOG_INFO("Handling new request");
	std::pair<Request, std::forward_list<std::string>> infos = Parser::parse(str); // parse the request
	Request* req = &infos.first;
	std::forward_list<std::string>* infoHashes = &infos.second;
	bool gzip = false;
	try { // check if the client accepts gzip
		if (req->at("accept-encoding").find("gzip") != std::string::npos)
			gzip = true;
	} catch (const std::exception& e) {}
	std::string check = Parser::check(*req); // check if we have all we need to process (saves time if not the case
	if (check != "success") { // missing params
		LOG_WARNING("Couldn't parse request (" + check + ")");
		return error(check, gzip);
	}
	if (Config::get("type") == "private" && req->at("action") == "update" && req->at("passkey") == Config::get("updatekey"))
		return update(req, infoHashes);
	User* u = getUser(req->at("passkey"));
	if (Config::get("type") == "private" && u == nullptr) {
		LOG_WARNING("Passkey " + req->at("passkey") + " not found");
		return error("passkey not found", gzip);
	}
	if (infoHashes->begin() == infoHashes->end()) {
		LOG_WARNING("Missing info hash");
		return error("missing info_hash", gzip);
	}
	if (req->find("ip") == req->end())
		req->emplace("ip", Utility::ip_hex_decode(ip));
	if (bannedIPs.find(req->at("ip")) != bannedIPs.end())
		return error("banned ip", gzip);
	if (u->isRestricted(req->at("ip")))
		return error("ip not associated with account");
	if (req->at("action") == "announce") {
		req->emplace("event", "updating");
		return announce(req, infoHashes->front(), gzip);
	}
	else if (req->at("action") == "scrape")
		return scrape(infoHashes, gzip);
	LOG_ERROR("Unexpected! Action not found");
	return error("invalid action", gzip); // not possible, since the request is checked, but, well, who knows :3
}

std::string RequestHandler::announce(const Request* req, const std::string& infoHash, bool gzip)
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
		torMap.emplace(infoHash, Torrent(0, 0, 0, 0));
	Torrent *tor = nullptr;
	try {
		tor = &torMap.at(infoHash);
	} catch (const std::exception& e) {
		LOG_WARNING("Torrent not found");
		return error("torrent not found", gzip);
	}
	Peers *peers = nullptr;
	Peer *peer = nullptr;
	unsigned int corrupt;
	unsigned long balance;
	try {
		corrupt = std::stoul(req->at("corrupt"));
	} catch (const std::exception& e) {
		corrupt = 0;
	}
	if (req->at("left") != "0") {
		peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
		if (peer == nullptr) {
			peer = tor->getLeechers()->addPeer(*req, tor->getID(), now);
			tor->change();
		}
		if (req->at("event") == "stopped")
			peer->inactive();
		if (Config::get("type") == "private") {
			int free = tor->getFree();
			if (leechStatus == FREELEECH)
				free = 100;
			if (peer->getUser()->hasToken(tor->getID())) {
				bool expired = peer->getUser()->isTokenExpired(tor->getID());
				db->recordToken(peer->getUser()->getID(), tor->getID(), peer->getTotalStats()-std::stoul(req->at("downloaded")), expired);
				free = 100;
			}
			balance = peer->getCorrupt()-corrupt;
			if (balance > 0)
				tor->decBalance(balance);
			peer->updateStats(std::stoul(req->at("downloaded"))*(1-(free/100)), std::stoul(req->at("left")), corrupt, now);
			balance = peer->getStats();
			if (balance > 0)
				tor->decBalance(balance);
		}
		db->recordPeer(peer);
		db->recordUser(peer->getUser());
		if (req->at("event") == "stopped") {
			tor->getLeechers()->removePeer(*req);
			tor->change();
		}
		peers = tor->getSeeders();
	} else {
		peer = tor->getSeeders()->getPeer(req->at("peer_id"), now);
		if (peer == nullptr) {
			peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
			if (peer == nullptr) {
				peer = tor->getSeeders()->addPeer(*req, tor->getID(), now);
				tor->change();
			}
		}
		if (req->at("event") == "stopped")
		   peer->inactive();
		if (Config::get("type") == "private") {
			if (req->at("event") == "completed") {
				int free = tor->getFree();
				if (leechStatus == FREELEECH)
					free = 100;
				if (peer->getUser()->hasToken(tor->getID())) {
					bool expired = peer->getUser()->isTokenExpired(tor->getID());
					db->recordToken(peer->getUser()->getID(), tor->getID(), peer->getTotalStats()-std::stoul(req->at("downloaded")), expired);
					free = 100;
				}
				balance = peer->getCorrupt()-corrupt;
				if (balance > 0)
					tor->decBalance(balance);
				peer->updateStats(std::stoul(req->at("downloaded"))*(1-(free/100)), std::stoul(req->at("left")), corrupt, now);
				balance = peer->getStats();
				if (balance > 0)
					tor->decBalance(balance);
			} else {
				peer->updateStats(std::stoul(req->at("uploaded")), std::stoul(req->at("left")), corrupt, now);
				balance = peer->getStats();
				if (balance > 0)
					tor->incBalance(balance);
			}
		}
		db->recordPeer(peer);
		db->recordUser(peer->getUser());
		if (req->at("event") == "stopped") {
			tor->getSeeders()->removePeer(*req);
			tor->change();
		}
		if (req->at("event") == "completed") {
			tor->getLeechers()->removePeer(*req);
			tor->change();
		}
		peers = tor->getLeechers();
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
			+ "e"),
			gzip
			);
}

std::string RequestHandler::scrape(const std::forward_list<std::string>* infoHashes, bool gzip)
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

	return response(resp, gzip);
}

std::string RequestHandler::update(const Request* req, const std::forward_list<std::string>* infoHashes)
{
	LOG_INFO("Update request");
	std::string resp = "failure";
	const std::string& type = req->at("type");

	if (type == "update_user")
		resp = changePasskey(req);
	else if (type == "add_torrent")
		resp = addTorrent(req, infoHashes->front());
	else if (type == "delete_torrent")
		resp = deleteTorrent(infoHashes->front());
	else if (type == "update_torrent")
		resp = updateTorrent(req, infoHashes->front());
	else if (type == "add_user")
		resp = addUser(req);
	else if (type == "remove_user")
		resp = removeUser(req);
	else if (type == "add_token")
		resp = addToken(req, infoHashes->front());
	else if (type == "remove_token")
		resp = removeToken(req, infoHashes->front());
	else if (type == "add_ban")
		resp = addBan(req);
	else if (type == "remove_ban")
		resp = removeBan(req);
	else if (type == "add_ip_restriction")
		resp = addIPRestriction(req);
	else if (type == "remove_ip_restriction")
		resp = removeIPRestriction(req);
	else if (type == "add_whitelist")
		resp = addWhitelist(req);
	else if (type == "remove_whitelist")
		resp = removeWhitelist(req);
	else if (type == "set_leech_status")
		resp = setLeechStatus(req);

	LOG_INFO(type + " : " + resp);
	return response(resp, false);
}

std::string RequestHandler::addIPRestriction(const Request* req)
{
	try {
		return (usrMap.at(req->at("passkey"))->addIPRestriction(req->at("ip"), Config::getInt("max_ip")) ? "success" : "failure");
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::removeIPRestriction(const Request* req)
{
	try {
		usrMap.at(req->at("passkey"))->removeIPRestriction(req->at("ip"));
		return "success";
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::addToken(const Request* req, const std::string& infoHash)
{
	try {
		usrMap.at(req->at("userpasskey"))->addToken(torMap.at(infoHash).getID());
		return "success";
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::removeToken(const Request* req, const std::string& infoHash)
{
	try {
		usrMap.at(req->at("userpasskey"))->removeToken(torMap.at(infoHash).getID());
		return "success";
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::changePasskey(const Request* req)
{
	try {
		const std::string& old = req->at("oldpasskey");
		User* user = usrMap.at(old);
		usrMap.erase(old);
		usrMap.emplace(req->at("newpasskey"), user);

		return "success";
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::addTorrent(const Request* req, const std::string& infoHash)
{
	try {
		auto t = torMap.emplace(infoHash, Torrent(std::stoul(req->at("id")), std::stoul(req->at("size")), 0, 0));
		if (!t.second)
			return "failure";
		try {
			if (req->at("freetorrent") == "1")
				t.first->second.setFree(100);
			else
				t.first->second.setFree(0);
		}
		catch (const std::exception& e) {}
		return "success";
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::deleteTorrent(const std::string& infoHash)
{
	const auto it = torMap.find(infoHash);
	if (it != torMap.end())
		torMap.erase(it);
	return "success";
}

std::string RequestHandler::updateTorrent(const Request* req, const std::string& infoHash)
{
	auto it = torMap.find(infoHash);
	if (it != torMap.end()) {
		if (req->at("freetorrent") == "1")
			it->second.setFree(100);
		else
			it->second.setFree(0);
		return "success";
	}
	return "failure";
}

std::string RequestHandler::addUser(const Request* req)
{
	try {
		return (usrMap.emplace(req->at("passkey"), new User(std::stoul(req->at("id")), true)).second) ? "success" : "failure";
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::removeUser(const Request* req)
{
	return (usrMap.erase(req->at("passkey")) == 1) ? "success" : "failure";
}

std::string RequestHandler::addBan(const Request* req) {
	try {
		unsigned int from = std::stoul(req->at("fromip"));
		unsigned int to = std::stoul(req->at("toip"));
		while (from != to)
			bannedIPs.emplace(Utility::long2ip(from++));
		bannedIPs.emplace(Utility::long2ip(from));
		return "success";
	} catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::removeBan(const Request* req) {
	try {
		unsigned int from = std::stoul(req->at("fromip"));
		unsigned int to = std::stoul(req->at("toip"));
		while (from != to)
			bannedIPs.erase(Utility::long2ip(from++));
		bannedIPs.erase(Utility::long2ip(from));
		return "success";
	} catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::addWhitelist(const Request* req) {
	try {
		std::string peerID = req->at("peer_id");
		clientWhitelist.push_back(peerID);
		return "success";
	} catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::removeWhitelist(const Request* req) {
	try {
		std::string peerID = req->at("peer_id");
		clientWhitelist.remove(peerID);
		return "success";
	} catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::setLeechStatus(const Request* req) {
	try {
		leechStatus = (req->at("leech_status") == "freeleech" ? FREELEECH : NORMAL);
		return "success";
	} catch (const std::exception& e) {
		return "failure";
	}
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
	LOG_INFO("Cleaning torrent peers");
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
