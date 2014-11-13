#include "requestHandler.hpp"
#include "response.hpp"
#include "utility.hpp"
#include <iostream>
#include <ctime>

torrentMap RequestHandler::torMap;

std::string RequestHandler::handle(std::string str, std::string ip)
{
	request req = Parser::parse(str);
	bool gzip = (req.at("accept-encoding").find("gzip") != std::string::npos);
	std::string check = Parser::check(req);
	if (check != "success")
		return error(check, gzip);
	//if ( Config::get("type") == "private" ) { // private tracker
	if (false) {
		if (req.find("passkey") == req.end())
			return error("passkey not found", gzip);
	}
	if (req.find("ip") == req.end())
		req.emplace("ip", ip);
	req.emplace("hex_ip", (Utility::ip_hex_encode(req.at("ip")) + Utility::port_hex_encode(req.at("port"))));
	if (req.find("hex_ip") == req.end())
		return error("invalid ip", gzip);
	if (req.at("action") == "announce")
		return announce(req);
	return error("invalid action", gzip);
}

std::string RequestHandler::announce(const request& req)
{
	torMap.emplace(req.at("info_hash"), std::pair<peerMap, peerMap>());
	peerMap pmap;
	if (std::stoi(req.at("left")) > 0) {
		torMap.at(req.at("info_hash")).second.emplace(req.at("hex_ip"), new User());
		pmap = torMap.at(req.at("info_hash")).first;
	} else {
		torMap.at(req.at("info_hash")).first.emplace(req.at("hex_ip"), new User());
		pmap = torMap.at(req.at("info_hash")).second;
	}
	std::string peers;
	//int i = std::stoi(req.at("numwant"));
	for ( auto it : pmap ) {
	//	if (i-- == 0)
	//		break;
		peers.append(it.first);
	}
	std::string resp;
	resp += "d8:completei0";
        resp += "e10:downloadedi0";
        resp += "e10:incompletei0";
        resp += "e8:intervali900";
        resp += "e12:min intervali300";
        resp += "e5:peers";
	if(peers.empty()) {
		resp += "0";
	} else {
		resp += std::to_string(peers.length());
		resp += ":";
		resp += peers;
	}
	resp += "e";
	return response(resp, (req.at("accept-encoding").find("gzip") != std::string::npos)); // doesn't look as bad as it is stated on ocelot, needs stresstesting to check
}
