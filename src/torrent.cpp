#include <iostream>
#include "torrent.hpp"

Torrent::Torrent (unsigned int id, unsigned char free, unsigned int downloaded) {
	this->id = id;
	this->free = free;
	this->downloaded = downloaded;
}

unsigned int Torrent::getID () {
	return this->id;
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
	++this->downloaded;
}

unsigned char Torrent::getFree() {
	return this->free;
}

void Torrent::setFree(unsigned char free) {
	this->free = free;
}
