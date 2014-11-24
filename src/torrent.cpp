#include "torrent.hpp"

Torrent::Torrent (unsigned int id) {
	this->id = id;
	this->downloaded = 0;
}

unsigned int Torrent::GetID () {
	return this->id;
}

Peers* Torrent::Seeders() {
	return &this->seeders;
}

Peers* Torrent::Leechers() {
	return &this->leechers;
}

unsigned int Torrent::Downloaded() {
	return downloaded;
}

void Torrent::Downloadedpp() {
	++downloaded;
}
