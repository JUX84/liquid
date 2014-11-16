#include "user.hpp"

void User::addHexIP(std::string hexIP) {
	this->hexIP.push_front(hexIP);
}

std::forward_list<std::string>* User::getHexIP() {
	return &this->hexIP;
}
