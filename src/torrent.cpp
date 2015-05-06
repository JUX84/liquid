#include <iostream>
#include "logger.hpp"
#include "torrent.hpp"

Torrent::Torrent (unsigned int id, unsigned long size, unsigned char free) {
	this->id = id;
	this->size = size;
	this->free = free;
	snatches = 0;
}

unsigned int Torrent::getID () {
	return id;
}

unsigned int Torrent::getSize () {
	return size;
}

Peers* Torrent::getSeeders() {
	return &seeders;
}

Peers* Torrent::getLeechers() {
	return &leechers;
}

unsigned int Torrent::getSnatches() {
	return snatches;
}

void Torrent::incSnatches() {
	LOG_INFO("New snatch on torrent " + std::to_string(id));
	++snatches;
}

unsigned char Torrent::getFree() {
	return free;
}

void Torrent::setFree(unsigned char free) {
	this->free = free;
}

bool Torrent::canRecord(long long now) {
	bool b = (lastUpdate < (now - 60));
	lastUpdate = now;
	return b;
}
