#include <chrono>
#include <iostream>
#include "config.hpp"
#include "peers.hpp"

Peer::Peer(std::string hexIP, class User* u, bool seeding, unsigned int fid, std::string client) {
	std::cout << "Creating peer on torrent " << std::to_string(fid) << " using client " << client << " (" << hexIP << ")\n";
	this->hexIP = hexIP;
	this->user = u;
	this->seeding = seeding;
	this->fid = fid;
	this->client = client;
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
	std::cout << "Updating stats (" << std::to_string(this->stats) << " -> " << std::to_string(stats) << ") on a user from torrent " << std::to_string(fid) << " (" << hexIP << ")\n";
	this->stats += stats;
	seedtime += now - lastUpdate;
	lastUpdate = now;
}

void Peer::resetStats() {
	this->stats = 0;
}

std::string Peer::record(const unsigned int& left, const std::string& peerID) {
	std::cout << "Recording stats of peer " << peerID << " (left: " << left << ")\n";
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
					"'" + std::to_string(fid) + "') ON DUPLICATE KEY UPDATE downloaded = downloaded + VALUES(downloaded), uploaded = uploaded + VALUES(uploaded), seedtime = seedtime + VALUES(seedtime)";
	user->updateStats(downloaded,uploaded);
}

std::string Peer::remove(const std::string& peerID, const unsigned int& fid) {
	std::cout << "Removing peer " << peerID << " from torrent " << std::to_string(fid) << '\n';
	return "DELETE FROM xbt_files_users WHERE peer_id LIKE '"
		+ peerID
		+ "' AND fid = "
		+ std::to_string(fid);
}

bool Peer::timedOut(const long long &now) {
	std::cout << "A PEER TIMED OUT\n";
	return (now - lastUpdate > Config::getInt("timeout"));
}
