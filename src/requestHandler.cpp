#include <algorithm>
#include <iostream>
#include <stdexcept>
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
	if (req->at("action") == "announce")
		return announce(req, infoHashes->front(), gzip);
	else if (req->at("action") == "scrape")
		return scrape(infoHashes, gzip);
	return error("invalid action", gzip); // not possible, since the request is checked, but, well, who knows :3
}

std::string RequestHandler::announce(const Request* req, const std::string& infoHash, const bool& gzip)
{
	if (Config::get("type") != "private")
		torMap.emplace(infoHash, Torrent(0));
	Torrent *tor = nullptr;
	try {
		tor = &torMap.at(infoHash);
	} catch (const std::exception& e) {
		return error("unregistered torrent", gzip);
	}
	Peers *peers = nullptr;
	if (req->at("left") != "0") {
		if (tor->getLeechers()->getPeer(req->at("peer_id")) == nullptr)
			tor->getLeechers()->addPeer(*req, tor->getID());
		else if (req->at("event") == "stopped")
			tor->getLeechers()->removePeer(*req);
		peers = tor->getSeeders();
	} else {
		if (tor->getSeeders()->getPeer(req->at("peer_id")) == nullptr)
			tor->getSeeders()->addPeer(*req, tor->getID());
		else if (req->at("event") == "stopped" || req->at("event") == "completed") {
			tor->getLeechers()->removePeer(*req);
			if (req->at("event") == "completed") {
				tor->downloadedpp();
				tor->getSeeders()->addPeer(*req, tor->getID());
			}
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
		peerlist.append(*peers->nextPeer()->getHexIP());
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
		resp = change_passkey(req);

	return resp;
}

std::string RequestHandler::change_passkey(const Request* req)
{
	try {
		const std::string& old = req->at("oldpasskey");
		User* user = usrMap.at(old);
		usrMap.erase(old);
		usrMap.emplace(req->at("newpasskey"), user);

		return "succes";
	}
	catch (const std::exception& e) {
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
	db = new MySQL();
	db->connect();
	db->loadUsers(usrMap);
	db->loadTorrents(torMap);
}

void RequestHandler::stop() {
	// TODO record every changes before flushing
	db->flush();
	db->disconnect();
}
