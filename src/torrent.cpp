#include "torrent.hpp"

Torrent::Torrent (unsigned int id) {
	this->id = id;
	this->downloaded = 0;
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
	return downloaded;
}

void Torrent::downloadedpp() {
	++downloaded;
}
