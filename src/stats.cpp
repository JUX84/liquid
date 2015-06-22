#include "stats.hpp"

unsigned long Stats::peersCount;

void Stats::incPeersCount() {
	++peersCount;
}

void Stats::decPeersCount() {
	--peersCount;
}

unsigned long Stats::getPeersCount() {
	return peersCount;
}
