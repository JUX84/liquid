#include <algorithm>
#include <iostream>
#include <stdexcept>
#include "config.hpp"
#include "requestHandler.hpp"
#include "response.hpp"
#include "torrent.hpp"
#include "mysql.hpp"

torrentMap RequestHandler::torMap;
userMap RequestHandler::usrMap;
Database* RequestHandler::db;

std::string RequestHandler::handle(std::string str, std::string ip)
{
	request req = Parser::parse(str); // parse the request
	try { // check if the client accepts gzip
		if (req.at("accept-encoding").find("gzip") != std::string::npos)
			req.emplace("gzip", "true");
	} catch (const std::exception& e) {}
	req.emplace("gzip", "false");
	req.erase("accept-encoding"); // not used anymore
	std::string check = Parser::check(req); // check if we have all we need to process (saves time if not the case
	if (check != "success") // missing params
		return error(check, req.at("gzip") == "true");
	if (Config::get("type") == "private" && getUser(req.at("passkey")) == nullptr)
		return error("passkey not found", req.at("gzip") == "true");
	req.emplace("ip", ip); // if an IP wasn't provided in the params
	if (req.at("action") == "announce")
		return announce(req);
	return error("invalid action", req.at("gzip") == "true"); // not possible, since the request is checked, but, well, who knows :3
}

std::string RequestHandler::announce(const request& req)
{
	torMap.emplace(req.at("info_hash"), Torrent());
	Torrent *tor = &torMap.at(req.at("info_hash"));
	PeerMap *pmap = nullptr;
	if (std::stoi(req.at("left")) > 0) {
		if (tor->Leechers()->getPeer(req.at("peer_id")) == nullptr)
			tor->Leechers()->addPeer(req);
		pmap = tor->Seeders();
	} else {
		if (tor->Seeders()->getPeer(req.at("peer_id")) == nullptr)
			tor->Seeders()->addPeer(req);
		pmap = tor->Leechers();
	}
	std::string peers;
	unsigned long i = 10;
	try {
		i = std::stoi(req.at("numwant"));
	} catch (const std::exception& e) {}
	i = std::min(i, pmap->size());
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
			req.at("gzip") == "true"
		       ); // doesn't look as bad as it is stated on ocelot, needs stresstesting to check
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
}
