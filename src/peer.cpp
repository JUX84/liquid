#include <chrono>
#include "config.hpp"
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
	seedtime = 0;
}

User* Peer::User() {
	return this->user;
}

std::string* Peer::getHexIP() {
	return &this->hexIP;
}

void Peer::updateStats (unsigned long stats, const long long &now) {
	this->stats += stats;
	seedtime += now - lastUpdate;
	lastUpdate = now;
}

void Peer::resetStats() {
	this->stats = 0;
}

std::string Peer::record(const unsigned int& left, const std::string& peerID) {
	unsigned int downloaded,uploaded = 0;
	auto time_point = std::chrono::system_clock::now();
	auto duration = time_point.time_since_epoch();
	auto now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	if (seeding) {
		uploaded = stats;
		downloaded = 0;
		seedtime = now - lastUpdate;
	} else {
		downloaded = stats;
		uploaded = 0;
	}
	stats = 0;
	lastUpdate = now;
	return "INSERT INTO xbt_files_users(uid,downloaded,uploaded,remaining,seedtime,useragent,peerid,fid) VALUES ('"
						+ std::to_string(*user->getID()) + "', " +
					"'" + std::to_string(downloaded) + "', " +
					"'" + std::to_string(uploaded) + "', " +
					"'" + std::to_string(left) + "', " +
					"'" + std::to_string(seedtime) + "', " +
					"'" + client + "', " +
					"'" + peerID + "', " +
					"'" + std::to_string(fid) + "') ON DUPLICATE downloaded = downloaded + VALUES(downloaded), uploaded = uploaded + VALUES(uploaded), seedtime = seedtime + VALUES(seedtime)";
	user->updateStats(downloaded,uploaded);
}

std::string Peer::remove(const std::string& peerID, const unsigned int& fid) {
	return "DELETE FROM xbt_files_users WHERE peer_id LIKE '"
		+ peerID
		+ "' AND fid = "
		+ std::to_string(fid);
}

bool Peer::timedOut(const long long &now) {
	return (now - lastUpdate > Config::getInt("timeout"));
}
