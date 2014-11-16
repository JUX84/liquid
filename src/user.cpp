#include "user.hpp"
#include "utility.hpp"

void User::addHexIP(const request& req) {
	this->hexIP.emplace(req.at("ip")+":"+req.at("port"),
			Utility::ip_hex_encode(req.at("ip"))
			+
			Utility::port_hex_encode(req.at("port"))
			);
}

std::unordered_map<std::string, std::string>* User::getHexIP() {
	return &this->hexIP;
}
