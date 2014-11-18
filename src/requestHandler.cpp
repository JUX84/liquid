#include <algorithm>
#include <iostream>
#include <stdexcept>
#include "config.hpp"
#include "requestHandler.hpp"
#include "response.hpp"
#include "torrent.hpp"
#include "utility.hpp"
#include "mysql.hpp"

torrentMap RequestHandler::torMap;
userMap RequestHandler::usrMap;
Database* RequestHandler::db;

std::string RequestHandler::handle(std::string str, std::string ip)
{
	request req = Parser::parse(str); // parse the request
	try { // check if the client accepts gzip
		if (req.first.at("accept-encoding").find("gzip") != std::string::npos)
			req.first.emplace("gzip", "true");
	} catch (const std::exception& e) {}
	req.first.emplace("gzip", "false");
	req.first.erase("accept-encoding"); // not used anymore
	std::string check = Parser::check(req); // check if we have all we need to process (saves time if not the case
	if (check != "success") // missing params
		return error(check, req.first.at("gzip") == "true");
	if (Config::get("type") == "private" && getUser(req.first.at("passkey")) == nullptr)
		return error("passkey not found", req.first.at("gzip") == "true");
	req.first.emplace("ip", ip); // if an IP wasn't provided in the params
	if (req.first.at("action") == "announce")
		return announce(req);
	else if (req.first.at("action") == "scrape")
		return scrape(req.second);
	return error("invalid action", req.first.at("gzip") == "true"); // not possible, since the request is checked, but, well, who knows :3
}

std::string RequestHandler::announce(const request& req)
{
	if (Config::get("type") != "private")
		torMap.emplace(req.second.front(), Torrent());
	Torrent *tor = nullptr;
	try {
		tor = &torMap.at(req.second.front());
	} catch (const std::exception& e) {
		return error("unregistered torrent", req.first.at("gzip") == "true");
	}
	PeerMap *pmap = nullptr;
	if (req.first.at("left") != "0") {
		if (tor->Leechers()->getPeer(req.first.at("peer_id")) == nullptr)
			tor->Leechers()->addPeer(req);
		else if (req.first.at("event") == "stopped")
			tor->Leechers()->removePeer(req);
		pmap = tor->Seeders();
	} else {
		if (tor->Seeders()->getPeer(req.first.at("peer_id")) == nullptr)
			tor->Seeders()->addPeer(req);
		else if (req.first.at("event") == "stopped")
			tor->Leechers()->removePeer(req);
		pmap = tor->Leechers();
	}
	std::string peers;
	unsigned long i = 0;
	if (req.first.at("event") != "stopped") {
		i = 10;
		try {
			i = std::stoi(req.first.at("numwant"));
		} catch (const std::exception& e) {}
		i = std::min(i, pmap->size());
	}
	while (i-- > 0) {
		for ( auto it : *pmap->nextPeer()->getHexIP())
			peers.append(it.second);
	}
	return response(
			("d8:completei"
			 + std::to_string(tor->Seeders()->size())
			 + "e10:incompletei"
			 + std::to_string(tor->Leechers()->size())
			 + "e8:intervali"
			 + std::to_string(900)
			 + "e12:min intervali"
			 + std::to_string(300)
			 + "e5:peers"
			 + std::to_string(peers.length())
			 + ":"
			 + peers
			 + "e"),
			req.first.at("gzip") == "true"
			); // doesn't look as bad as it is stated on ocelot, needs stresstesting to check
}

std::string RequestHandler::scrape(const std::forward_list<std::string>& infoHashes)
{
	std::string response("d5:files");

	for (const auto& infoHash : infoHashes) {
		const torrentMap::iterator it = torMap.find(infoHash);
		if (it != torMap.end()) {
			response += "d20:" + infoHash
				+ "d8:completei" + std::to_string(it->second.Seeders()->size()) + 'e'
				+ "10:downloadedi0e"
				+ "10:incompletei" + std::to_string(it->second.Leechers()->size()) + "eee";
		}
	}
	response += "e";

	return response;
}

User* RequestHandler::getUser(const std::string& passkey) {
	try {
		return &usrMap.at(passkey);
	} catch (const std::exception& e) {
		return nullptr;
	}
}

void RequestHandler::init() {
	db = new MySQL();
	db->LoadUsers(usrMap);
	db->LoadTorrents(torMap);
}
