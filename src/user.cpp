#include "user.hpp"

bool User::isSetHexIP() {
	return !this->hexIP.empty();
}

void User::setHexIP(std::string hexIP) {
	this->hexIP = hexIP;
}

std::string User::getHexIP() {
	return this->hexIP;
}
