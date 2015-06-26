#include "misc/logger.hpp"
#include "misc/stats.hpp"
#include "misc/utility.hpp"

bool Stats::changed;
unsigned long Stats::peers;
unsigned long Stats::torrents;
unsigned long Stats::speed;
double Stats::speedShow;
unsigned char Stats::speedLvl;
unsigned long Stats::transferred;
double Stats::transferredShow;
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
	speedShow = speed;
	speedLvl = 0;
	while (speedShow > 1024 && speedLvl < 9) {
		speedShow /= 1024;
		++speedLvl;
	}
	changed = true;
}

void Stats::decSpeed(unsigned long decspeed) {
	speed -= decspeed;
}

void Stats::incTransferred(unsigned long inctransferred) {
	transferred += inctransferred;
	transferredShow = transferred;
	transferredLvl = 0;
	while (transferredShow > 1024 && transferredLvl < 9) {
		transferredShow /= 1024;
		++transferredLvl;
	}
	changed = true;
}

void Stats::show(ev::timer& timer, int revents) {
	if (changed) {
		LOG_INFO("Stats - " +
				std::to_string(peers) + " active peers on " +
				std::to_string(torrents) + " torrents - " +
				std::to_string(speedShow) + " " + Utility::getPrefix(speedLvl) + "B/s - " +
				std::to_string(transferredShow) + " " + Utility::getPrefix(transferredLvl) + "B transferred since start");
		changed = false;
	}
}
