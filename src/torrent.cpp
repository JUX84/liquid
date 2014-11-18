#include "torrent.hpp"

Peers* Torrent::Seeders() {
	return &this->seeders;
}

Peers* Torrent::Leechers() {
	return &this->leechers;
}
