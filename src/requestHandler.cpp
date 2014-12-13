#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include "config.hpp"
#include "requestHandler.hpp"
#include "response.hpp"
#include "torrent.hpp"
#include "utility.hpp"
#include "mysql.hpp"

TorrentMap RequestHandler::torMap;
UserMap RequestHandler::usrMap;
Database* RequestHandler::db;

std::string RequestHandler::handle(std::string str, std::string ip)
{
	std::pair<Request, std::forward_list<std::string>> infos = Parser::parse(str); // parse the request
	Request* req = &infos.first;
	std::forward_list<std::string>* infoHashes = &infos.second;
	bool gzip = false;
	try { // check if the client accepts gzip
		if (req->at("accept-encoding").find("gzip") != std::string::npos)
			gzip = true;
	} catch (const std::exception& e) {}
	std::string check = Parser::check(*req); // check if we have all we need to process (saves time if not the case
	if (infoHashes->begin() == infoHashes->end())
		return error("missing info_hash", gzip);
	if (check != "success") // missing params
		return error(check, gzip);
	if (Config::get("type") == "private" && getUser(req->at("passkey")) == nullptr)
		return error("passkey not found", gzip);
	try {
		req->at("ip") = Utility::ip_hex_encode(req->at("ip")) + Utility::port_hex_encode(req->at("port"));
	} catch (const std::exception& e) {
		req->emplace("ip", ip + Utility::port_hex_encode(req->at("port")));
	}
	if (req->at("action") == "announce") {
		req->emplace("event", "updating");
		return announce(req, infoHashes->front(), gzip);
	}
	else if (req->at("action") == "scrape")
		return scrape(infoHashes, gzip);
	return error("invalid action", gzip); // not possible, since the request is checked, but, well, who knows :3
}

std::string RequestHandler::announce(const Request* req, const std::string& infoHash, const bool& gzip)
{
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	if (Config::get("type") != "private")
		torMap.emplace(infoHash, Torrent(0));
	Torrent *tor = nullptr;
	try {
		tor = &torMap.at(infoHash);
	} catch (const std::exception& e) {
		return error("unregistered torrent", gzip);
	}
	Peers *peers = nullptr;
	Peer *peer = nullptr;
	if (req->at("left") != "0") {
		peer = tor->getLeechers()->getPeer(req->at("peer_id"), now);
		if (req->at("event") == "stopped") {
			if (peer != nullptr) {
				db->record(peer->record(std::stoul(req->at("left"))));
				db->record(peer->remove());
				tor->getLeechers()->removePeer(*req);
			}
		} else if (req->at("event") == "started") {
			if (peer == nullptr)
				tor->getLeechers()->addPeer(*req, tor->getID(), now);
		} else if (peer != nullptr) {
			peer->updateStats(std::stoul(req->at("downloaded"))*(1-(tor->getFree()/100)), now);
			db->record(peer->record(std::stoul(req->at("left"))));
		}
		peers = tor->getSeeders();
	} else {
		peer = tor->getSeeders()->getPeer(req->at("peer_id"), now);
		if (req->at("event") == "stopped" || req->at("event") == "completed") {
			if (peer != nullptr) {
				if (req->at("event") == "completed") {
					tor->downloadedpp();
					tor->getSeeders()->addPeer(*req, tor->getID(), now);
					tor->getLeechers()->removePeer(*req);
				} else {
					db->record(peer->record(std::stoul(req->at("left"))));
					db->record(peer->remove());
				}
			}
		} else if (req->at("event") == "started") {
			if (peer == nullptr)
				tor->getSeeders()->addPeer(*req, tor->getID(), now);
		} else if (peer != nullptr) {
			peer->updateStats(std::stoul(req->at("uploaded")), now);
			db->record(peer->record(std::stoul(req->at("left"))));
		}
		peers = tor->getLeechers();
	}
	std::string peerlist;
	unsigned long i = 0;
	if (req->at("event") != "stopped") {
		i = 10;
		try {
			i = std::stoi(req->at("numwant"));
		} catch (const std::exception& e) {}
		i = std::min(i, peers->size());
	}
	while (i-- > 0) {
		Peer* p = peers->nextPeer(now);
		if (p != nullptr)
			peerlist.append(*p->getHexIP());
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

std::string RequestHandler::scrape(const std::forward_list<std::string>* infoHashes, const bool& gzip)
{
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

std::string RequestHandler::update(const Request* req)
{
	std::string resp;
	const std::string& type = req->at("type");

	if (type == "change_passkey")
		resp = changePasskey(req);
	else if (type == "add_torrent")
		resp = addTorrent(req);
	else if (type == "delete_torrent")
		resp = deleteTorrent(req);
	else if (type == "update_torrent")
		resp = updateTorrent(req);
	else if (type == "add_user")
		resp = addUser(req);
	else if (type == "remove_user")
		resp = removeUser(req);

	return resp;
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

std::string RequestHandler::addTorrent(const Request* req)
{
	try {
		auto t = torMap.emplace(req->at("info_hash"), Torrent(std::stoul(req->at("id"))));
		if (!t.second)
			return "failure";

		try {
			if (req->at("freetorrent") == "1")
				t.first->second.setFree(1);
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

std::string RequestHandler::deleteTorrent(const Request* req)
{
	const auto it = torMap.find(req->at("info_hash"));
	if (it != torMap.end())
		torMap.erase(it);

	return "success";
}

std::string RequestHandler::updateTorrent(const Request* req)
{
	auto it = torMap.find(req->at("info_hash"));
	if (it != torMap.end()) {
		if (req->at("freetorrent") == "1")
			it->second.setFree(1);
		else
			it->second.setFree(0);

		return "success";
	}

	return "failure";
}

std::string RequestHandler::addUser(const Request* req)
{
	try {
		return (usrMap.emplace(req->at("passkey"), new User(std::stoul(req->at("id")))).second) ? "success" : "failure";
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
	db = new MySQL();
	db->connect();
	db->loadUsers(usrMap);
	db->loadTorrents(torMap);
}

void RequestHandler::stop() {
	// TODO record every changes before flushing
	if (Config::get("type") == "private") {
		db->flush();
		db->disconnect();
	}
}

void RequestHandler::clearTorrentPeers(ev::timer& timer, int revents)
{
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	for (auto& t : torMap) {
		t.second.getSeeders()->timedOut(now);
		t.second.getLeechers()->timedOut(now);
		//if(Config::get("type") == "public" && t.second.getSeeders()->size() == 0 && t.second.getLeechers()->size() == 0)
		// TODO: remove torrent from tormap
	}
}
