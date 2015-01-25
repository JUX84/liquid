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
std::forward_list<std::string> RequestHandler::bannedIPs;

std::string RequestHandler::handle(std::string str, std::string ip)
{
	LOG_INFO("Handling request (" + ip + ")\n" + str);
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
	if (Config::get("type") == "private" && getUser(req->at("passkey")) == nullptr) {
		LOG_WARNING("No passkey");
		return error("passkey not found", gzip);
	}
	if (req->at("action") == "update")
		return update(req, infoHashes);
	if (infoHashes->begin() == infoHashes->end()) {
		LOG_WARNING("Missing info hash");
		return error("missing info_hash", gzip);
	}
	try {
		req->at("ip") = Utility::ip_hex_encode(req->at("ip")) + Utility::port_hex_encode(req->at("port"));
	} catch (const std::exception& e) {
		req->emplace("ip", ip + Utility::port_hex_encode(req->at("port")));
	}
	for (const auto& bannedIP : bannedIPs) {
		if(bannedIP == req->at("ip"))
			return error("banned ip", gzip);
	}
	if (req->at("action") == "announce") {
		req->emplace("event", "updating");
		return announce(req, infoHashes->front(), gzip);
	}
	else if (req->at("action") == "scrape")
		return scrape(infoHashes, gzip);
	LOG_ERROR("WOWOWOW this shouldn't happen. NEVER !");
	return error("invalid action", gzip); // not possible, since the request is checked, but, well, who knows :3
}

std::string RequestHandler::announce(const Request* req, const std::string& infoHash, bool gzip)
{
	LOG_INFO("Announce request (" + infoHash + ")");
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	if (Config::get("type") != "private")
		torMap.emplace(infoHash, Torrent(0, 0, 0, 0));
	Torrent *tor = nullptr;
	try {
		tor = &torMap.at(infoHash);
	} catch (const std::exception& e) {
		LOG_WARNING("Torrent not found");
		return error("unregistered torrent", gzip);
	}
	Peers *peers = nullptr;
	Peer *peer = nullptr;
	if (req->at("left") != "0") {
		peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
		if (!peer->isSnatched() && (std::stoul(req->at("left")) < ((1-0.25)*tor->getSize())))
			peer->snatched();
		if (req->at("event") == "stopped") {
			if (peer != nullptr) {
				db->recordPeer(peer, std::stoul(req->at("left")), now);
				db->recordPeerRemoval(peer);
				if(peer->User()->canRecord(now))
					db->recordUser(peer->User());
				tor->getLeechers()->removePeer(*req);
			}
		} else if (req->at("event") == "started" || peer == nullptr) {
			if (peer == nullptr)
				tor->getLeechers()->addPeer(*req, tor->getID(), now);
		} else if (peer != nullptr) {
			int free = tor->getFree();
			if (peer->User()->hasToken(infoHash))
				free = 100;
			peer->updateStats(std::stoul(req->at("downloaded"))*(1-(tor->getFree()/100)), now);
			db->recordPeer(peer, std::stoul(req->at("left")), now);
			if(peer->User()->canRecord(now))
				db->recordUser(peer->User());
		}
		peers = tor->getSeeders();
	} else {
		peer = tor->getSeeders()->getPeer(req->at("peer_id"), now);
		if (req->at("event") == "stopped" || req->at("event") == "completed") {
			if (peer != nullptr) {
				peer->updateStats(std::stoul(req->at("uploaded")), now);
				db->recordPeer(peer, std::stoul(req->at("left")), now);
				db->recordPeerRemoval(peer);
				if(peer->User()->canRecord(now))
					db->recordUser(peer->User());
			} else if (req->at("event") == "completed") {
				tor->downloadedpp();
				tor->getLeechers()->removePeer(*req);
			}
		} else if (req->at("event") == "started" || peer == nullptr) {
			if (peer == nullptr)
				tor->getSeeders()->addPeer(*req, tor->getID(), now);
		} else if (peer != nullptr) {
			peer->updateStats(std::stoul(req->at("uploaded")), now);
			db->recordPeer(peer, std::stoul(req->at("left")), now);
			if(peer->User()->canRecord(now))
				db->recordUser(peer->User());
		}
		peers = tor->getLeechers();
	}
	LOG_INFO("Handled user stats");
	std::string peerlist;
	unsigned long i = 0;
	if (req->at("event") != "stopped") {
		i = 10;
		try {
			i = std::stoi(req->at("numwant"));
		} catch (const std::exception& e) {}
		i = std::min(i, peers->size());
	}
	LOG_INFO("creating peer list (" + std::to_string(i) + ")");
	while (i-- > 0) {
		Peer* p = peers->nextPeer(now);
		if (p != nullptr)
			peerlist.append(p->getHexIP());
	}
	return response(
			("d8:completei"
			 + std::to_string(tor->getSeeders()->size())
			 + "e10:incompletei"
			 + std::to_string(tor->getLeechers()->size())
			 + "e10:downloadedi"
			 + std::to_string(tor->getDownloaded())
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
			); // doesn't look as bad as it is stated on ocelot, needs stresstesting to check
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
				+ std::to_string(it->second.getDownloaded())
				+ "ee";
		}
	}
	resp += "ee";

	return response(resp, gzip);
}

std::string RequestHandler::update(const Request* req, const std::forward_list<std::string>* infoHashes)
{
	LOG_INFO("Update request (" + req->at("passkey") + ")");
	std::string resp;
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

	LOG_INFO(type + " : " + resp);
	return resp;
}

std::string RequestHandler::addToken(const Request* req, const std::string& infoHash)
{
	try {
		usrMap.at(req->at("userpasskey"))->addToken(infoHash);
		return "success";
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::removeToken(const Request* req, const std::string& infoHash)
{
	try {
		usrMap.at(req->at("userpasskey"))->removeToken(infoHash);
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
		return (usrMap.emplace(req->at("passkey"), new User(std::stoul(req->at("id")), true, true)).second) ? "success" : "failure";
	}
	catch (const std::exception& e) {
		return "failure";
	}
}

std::string RequestHandler::removeUser(const Request* req)
{
	return (usrMap.erase(req->at("passkey")) == 1) ? "success" : "failure";
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
}

void RequestHandler::stop() {
	LOG_INFO("Stopping request handler");
	if (Config::get("type") == "private") {
		db->flush();
		db->disconnect();
	}
}

void RequestHandler::clearTorrentPeers(ev::timer& timer, int revents)
{
	LOG_INFO("Cleaning torrent peers");
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	auto t = torMap.begin();
	while (t != torMap.end()) {
		t->second.getSeeders()->timedOut(now);
		t->second.getLeechers()->timedOut(now);
		if(Config::get("type") == "public" && t->second.getSeeders()->size() == 0 && t->second.getLeechers()->size() == 0)
			torMap.erase(t++);
		else
			++t;
	}
}

void RequestHandler::flushSqlRecords(ev::timer& timer, int revents) {
	db->flush();
}
