#include "torrent.hpp"

peerMap* Torrent::Seeders() {
	return &this->seeders;
}

peerMap* Torrent::Leechers() {
	return &this->leechers;
}

peerMap::iterator* Torrent::LastSeeder() {
	return &this->it;
}

peerMap::iterator* Torrent::LastLeecher() {
	return &this->it;
}
