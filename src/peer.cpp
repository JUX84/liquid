#include "peers.hpp"

Peer::Peer(std::string hexIP, User* u) {
	this->hexIP = hexIP;
	this->user = u;
}

User* Peer::getUser() {
	return this->user;
}

std::string* Peer::getHexIP() {
	return &this->hexIP;
}
