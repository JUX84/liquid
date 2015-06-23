#include "stats.hpp"
#include "utility.hpp"

unsigned long Stats::peers;
double Stats::speed;
unsigned char Stats::speedLvl;
double Stats::transferred;
unsigned char Stats::transferredLvl;

void Stats::incPeers() {
	++peers;
}

void Stats::decPeers() {
	--peers;
}

std::string Stats::getPeers() {
	return std::to_string(peers);
}

void Stats::incSpeed(unsigned long incspeed) {
	speed += incspeed;
	while (speed > 1024 && speedLvl < 9) {
		speed /= 1024;
		++speedLvl;
	}
}

void Stats::decSpeed(unsigned long decspeed) {
	speed -= decspeed;
	while (speed < 1024 && speedLvl > 0) {
		speed *= 1024;
		--speedLvl;
	}
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
}

std::string Stats::getTransferred() {
	return std::to_string(transferred) + " " + Utility::getPrefix(transferredLvl) + "B/s";
}
