#include "user.hpp"
#include "utility.hpp"

void User::addHexIP(const Request& req) {
	this->hexIP.emplace(req.first.at("ip")+":"+req.first.at("port"),
			Utility::ip_hex_encode(req.first.at("ip"))
			+
			Utility::port_hex_encode(req.first.at("port"))
			);
}

std::unordered_map<std::string, std::string>* User::getHexIP() {
	return &this->hexIP;
}
