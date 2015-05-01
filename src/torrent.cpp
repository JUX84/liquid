#include <iostream>
#include "logger.hpp"
#include "torrent.hpp"

Torrent::Torrent (unsigned int id, unsigned long size, unsigned char free, unsigned int downloaded) {
	this->id = id;
	this->size = size;
	this->free = free;
	this->downloaded = downloaded;
}

unsigned int Torrent::getID () {
	return this->id;
}

unsigned int Torrent::getSize () {
	return this->size;
}

Peers* Torrent::getSeeders() {
	return &this->seeders;
}

Peers* Torrent::getLeechers() {
	return &this->leechers;
}

unsigned int Torrent::getDownloaded() {
	return this->downloaded;
}

void Torrent::downloadedpp() {
	LOG_INFO("New snatch on torrent " + std::to_string(id));
	++this->downloaded;
}

unsigned char Torrent::getFree() {
	return this->free;
}

void Torrent::setFree(unsigned char free) {
	this->free = free;
}

bool Torrent::canRecord(long long now) {
	bool b = (lastUpdate < (now - 60));
	lastUpdate = now;
	return b;
}
