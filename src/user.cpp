#include "user.hpp"

bool User::isSet() {
	return !this->hex_ip.empty();
}

void User::set(std::string hex_ip) {
	this->hex_ip = hex_ip;
}

std::string User::get() {
	return this->hex_ip;
}
