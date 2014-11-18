#include "peers.hpp"

Peer::Peer(User* u) {
	this->user = u;
}

User* Peer::getUser() {
	return this->user;
}
