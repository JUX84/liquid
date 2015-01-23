#include <chrono>
#include <iostream>
#include "logger.hpp"
#include "config.hpp"
#include "peers.hpp"

Peer::Peer(std::string hexIP, class User* u, bool seeding, unsigned int fid, std::string client, std::string peerID) {
	LOG_INFO("Creating peer on torrent " + std::to_string(fid) + " using client " + client);
	this->hexIP = hexIP;
	this->user = u;
	this->seeding = seeding;
	this->fid = fid;
	this->client = client;
	this->peerID = peerID;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
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
	LOG_INFO("Updating stats ("  + std::to_string(stats) + ") on a peer on torrent " + std::to_string(fid));
	this->stats = stats;
	seedtime += now - lastUpdate;
	lastUpdate = now;
}

void Peer::resetStats() {
	this->stats = 0;
}

std::string Peer::record(const unsigned int& left) {
	LOG_INFO("Recording stats of peer " + peerID + " (left: " + std::to_string(left) + ")");
	unsigned int downloaded,uploaded = 0;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	auto now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	if (seeding) {
		uploaded = stats;
		downloaded = 0;
		seedtime = now - lastUpdate;
	} else {
		downloaded = stats;
		uploaded = 0;
	}
	std::string output = "INSERT INTO xbt_files_users(uid,downloaded,uploaded,remaining,seedtime,useragent,peer_id,fid) VALUES ('"
						+ std::to_string(*user->getID()) + "', " +
					"'" + std::to_string(downloaded) + "', " +
					"'" + std::to_string(uploaded) + "', " +
					"'" + std::to_string(left) + "', " +
					"'" + std::to_string(seedtime) + "', " +
					"'" + client + "', " +
					"'" + peerID + "', " +
					"'" + std::to_string(fid) + "') ON DUPLICATE KEY UPDATE downloaded = VALUES(downloaded), uploaded = VALUES(uploaded), seedtime = seedtime + VALUES(seedtime)";
	stats = 0;
	seedtime = 0;
	lastUpdate = now;
	user->updateStats(downloaded,uploaded,now);
	return output;
}

std::string Peer::remove() {
	LOG_INFO("Removing peer " + peerID + " on torrent " + std::to_string(fid));
	return "DELETE FROM xbt_files_users WHERE peer_id LIKE '"
		+ peerID
		+ "' AND fid = "
		+ std::to_string(fid);
}

bool Peer::timedOut(const long long &now) {
	return (now - lastUpdate > Config::getInt("timeout"));
}
