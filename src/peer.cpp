#include <chrono>
#include "peers.hpp"

Peer::Peer(std::string hexIP, class User* u, bool seeding, unsigned int fid, std::string client) {
	this->hexIP = hexIP;
	this->user = u;
	this->seeding = seeding;
	this->fid = fid;
	this->client = client;
	auto time_point = std::chrono::system_clock::now();
	auto duration = time_point.time_since_epoch();
	lastUpdate = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
}

User* Peer::User() {
	return this->user;
}

std::string* Peer::getHexIP() {
	return &this->hexIP;
}

void Peer::updateStats (unsigned long stats) {
	this->stats += stats;
}

void Peer::resetStats() {
	this->stats = 0;
}

std::string Peer::record() {
	unsigned int downloaded,uploaded;
	if (seeding) {
		uploaded = stats;
		downloaded = 0;
	} else {
		downloaded = stats;
		uploaded = 0;
	}
	stats = 0;
	auto time_point = std::chrono::system_clock::now();
	auto duration = time_point.time_since_epoch();
	auto now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	unsigned int seedtime = now - lastUpdate;
	lastUpdate = now;
	return "INSERT INTO liquid(uid,downloaded,uploaded,seedtime,client,fid) VALUES ('"
						+ std::to_string(*user->getID()) + "', " +
					"'" + std::to_string(downloaded) + "', " +
					"'" + std::to_string(uploaded) + "', " +
					"'" + std::to_string(seedtime) + "', " +
					"'" + client + "', " +
					"'" + std::to_string(fid) + "') ON DUPLICATE downloaded = downloaded + VALUES(downloaded), uploaded = uploaded + VALUES(uploaded), seedtime = seedtime + VALUES(seedtime)";
	user->updateStats(downloaded,uploaded);
}
