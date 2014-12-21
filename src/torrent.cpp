#include <iostream>
#include "torrent.hpp"

Torrent::Torrent (unsigned int id) {
	this->id = id;
	this->downloaded = 0;
	this->free = 0;
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
