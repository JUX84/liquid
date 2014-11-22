#include "peers.hpp"

Peer::Peer(std::string hexIP, class User* u) {
	this->hexIP = hexIP;
	this->user = u;
}

User* Peer::User() {
	return this->user;
}

std::string* Peer::HexIP() {
	return &this->hexIP;
}

void Peer::UpdateStats (unsigned long stats) {
	this->stats += stats;
}

void Peer::ResetStats() {
	this->stats = 0;
}

std::string Peer::Record() {
	return "";
}
