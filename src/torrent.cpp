#include <iostream>
#include "logger.hpp"
#include "torrent.hpp"

Torrent::Torrent (unsigned int id, unsigned char free, long balance) {
	this->id = id;
	this->free = free;
	this->balance = balance;
	snatches = 0;
	changed = false;
}

unsigned int Torrent::getID () {
	return id;
}

Peers* Torrent::getSeeders() {
	return &seeders;
}

Peers* Torrent::getSeeders6() {
	return &seeders;
}

Peers* Torrent::getLeechers() {
	return &leechers;
}

Peers* Torrent::getLeechers6() {
	return &leechers;
}

unsigned int Torrent::getSnatches() {
	return snatches;
}

void Torrent::incSnatches() {
	LOG_INFO("New snatch on torrent " + std::to_string(id));
	++snatches;
	changed = true;
}

void Torrent::reset() {
	snatches = 0;
	changed = false;
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
	changed = true;
	this->balance += balance;
}

void Torrent::change() {
	changed = true;
}

bool Torrent::hasChanged() {
	return changed;
}
