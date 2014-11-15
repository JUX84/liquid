#include "torrent.hpp"

peerMap* Torrent::Seeders() {
	return &this->seeders;
}

peerMap* Torrent::Leechers() {
	return &this->leechers;
}
