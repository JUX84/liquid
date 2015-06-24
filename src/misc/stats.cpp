#include "misc/stats.hpp"
#include "misc/utility.hpp"

bool Stats::changed;
unsigned long Stats::peers;
unsigned long Stats::torrents;
double Stats::speed;
unsigned char Stats::speedLvl;
double Stats::transferred;
unsigned char Stats::transferredLvl;

void Stats::reset() {
	changed = false;
}

bool Stats::hasChanged() {
	return changed;
}

void Stats::incPeers() {
	++peers;
	changed = true;
}

void Stats::decPeers() {
	--peers;
	changed = true;
}

std::string Stats::getPeers() {
	return std::to_string(peers);
}

void Stats::incTorrents() {
	++torrents;
	changed = true;
}

void Stats::decTorrents() {
	--torrents;
	changed = true;
}

void Stats::setTorrents(unsigned long torrentsCount) {
	torrents = torrentsCount;
}

unsigned long Stats::getTorrents() {
	return torrents;
}

std::string Stats::getTorrentsStr() {
	return std::to_string(torrents);
}

void Stats::incSpeed(unsigned long incspeed) {
	speed += incspeed;
	while (speed > 1024 && speedLvl < 9) {
		speed /= 1024;
		++speedLvl;
	}
	changed = true;
}

void Stats::decSpeed(unsigned long decspeed) {
	speed -= decspeed;
	while (speed < 1024 && speedLvl > 0) {
		speed *= 1024;
		--speedLvl;
	}
	changed = true;
}

std::string Stats::getSpeed() {
	return std::to_string(speed) + " " + Utility::getPrefix(speedLvl) + "B/s";
}

void Stats::incTransferred(unsigned long inctransferred) {
	transferred += inctransferred;
	while (transferred > 1024 && transferredLvl < 9) {
		transferred /= 1024;
		++transferredLvl;
	}
	changed = true;
}

std::string Stats::getTransferred() {
	return std::to_string(transferred) + " " + Utility::getPrefix(transferredLvl) + "B";
}
