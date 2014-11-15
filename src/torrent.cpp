#include "torrent.hpp"

peerMap* Torrent::Seeders() {
	return &this->seeders;
}

peerMap* Torrent::Leechers() {
	return &this->leechers;
}

void Torrent::LastSeeder(peerMap::iterator it) {
	this->it = it;
}

peerMap::iterator Torrent::LastSeeder() {
	return this->it;
}
