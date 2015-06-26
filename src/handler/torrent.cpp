#include "handler/torrent.hpp"
#include "misc/logger.hpp"

Torrent::Torrent (unsigned int id, unsigned char free, long balance) {
	this->id = id;
	this->free = free;
	this->balance = balance;
	snatches = 0;
}

unsigned int Torrent::getID () {
	return id;
}

Peers* Torrent::getSeeders() {
	return &seeders;
}

Peers* Torrent::getSeeders6() {
	return &seeders6;
}

Peers* Torrent::getLeechers() {
	return &leechers;
}

Peers* Torrent::getLeechers6() {
	return &leechers6;
}

unsigned int Torrent::getSnatches() {
	return snatches;
}

void Torrent::incSnatches() {
	LOG_INFO("New snatch on torrent " + std::to_string(id));
	++snatches;
}

void Torrent::reset() {
	snatches = 0;
}

unsigned char Torrent::getFree() {
	return free;
}

void Torrent::setFree(unsigned char free) {
	this->free = free;
}

long Torrent::getBalance() {
	return balance;
}

void Torrent::setBalance(long balance) {
	this->balance += balance;
}
