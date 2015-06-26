#include "misc/logger.hpp"
#include "misc/stats.hpp"
#include "misc/utility.hpp"

bool Stats::changed;
unsigned long Stats::peers;
unsigned long Stats::torrents;
double Stats::speed;
unsigned char Stats::speedLvl;
double Stats::transferred;
unsigned char Stats::transferredLvl;

void Stats::incPeers() {
	++peers;
	changed = true;
}

void Stats::decPeers() {
	--peers;
	changed = true;
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

void Stats::incTransferred(unsigned long inctransferred) {
	transferred += inctransferred;
	while (transferred > 1024 && transferredLvl < 9) {
		transferred /= 1024;
		++transferredLvl;
	}
	changed = true;
}

void Stats::show(ev::timer& timer, int revents) {
	if (changed) {
		LOG_INFO("Stats - " +
				std::to_string(peers) + " active peers on " +
				std::to_string(torrents) + " torrents - " +
				std::to_string(speed) + " " + Utility::getPrefix(speedLvl) + "B/s - " +
				std::to_string(transferred) + " " + Utility::getPrefix(transferredLvl) + "B transferred since start");
		changed = false;
	}
}
