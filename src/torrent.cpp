#include "torrent.hpp"

PeerMap* Torrent::Seeders() {
	return &this->seeders;
}

PeerMap* Torrent::Leechers() {
	return &this->leechers;
}
