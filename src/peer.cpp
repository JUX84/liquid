#include <chrono>
#include <iostream>
#include "logger.hpp"
#include "config.hpp"
#include "peers.hpp"
#include "utility.hpp"

Peer::Peer(std::string IP, std::string port, class User* u, bool seeding, unsigned long left, unsigned long stats, unsigned int torrentID, std::string client, std::string peerID) {
	LOG_INFO("Creating peer on torrent " + std::to_string(torrentID) + " using client " + client);
	user = u;
	total_stats = stats;
	stats = 0;
	this->left = left;
	this->seeding = seeding;
	completed = seeding;
	active = true;
	this->IP = IP;
	this->hexIPPort = Utility::ip_port_hex_encode(IP, port);
	this->peerID = peerID;
	this->torrentID = torrentID;
	this->client = client;
	timespent = 0;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	lastUpdate = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	this->speed = 0;
	changed = true;
}

User* Peer::getUser() {
	return user;
}

const std::string& Peer::getPeerID() {
	return peerID;
}

const std::string& Peer::getIP() {
	return IP;
}

const std::string& Peer::getHexIPPort() {
	return hexIPPort;
}

const std::string& Peer::getClient() {
	return client;
}

void Peer::updateStats (unsigned long stats, unsigned long left, unsigned int corrupt, long long now) {
	this->stats = stats - total_stats;
	total_stats = stats;
	this->left = left;
	if (now > lastUpdate)
		speed = (this->stats)/(now-lastUpdate);
	else
		speed = 0;
	if (seeding)
		timespent += now - lastUpdate;
	this->corrupt = corrupt;
	lastUpdate = now;
	if (this->stats > 0 || timespent > 0)
		changed = true;
}

unsigned long Peer::getLeft() {
	return left;
}

long long Peer::getLastUpdate() {
	return lastUpdate;
}

bool Peer::isSeeding() {
	return seeding;
}

bool Peer::isCompleted() {
	return completed;
}

unsigned long Peer::getStats() {
	return stats;
}

unsigned long Peer::getTotalStats() {
	return total_stats;
}

unsigned int Peer::getTorrentID() {
	return torrentID;
}

void Peer::snatched() {
	completed = true;
}

bool Peer::isSnatched() {
	return completed;
}

bool Peer::timedOut(long long now) {
	return (now - lastUpdate) > Config::getInt("timeout");
}

unsigned long Peer::getTimespent() {
	return timespent;
}

bool Peer::isActive() {
	return active;
}

void Peer::inactive() {
	active = false;
}

unsigned int Peer::getSpeed() {
	return speed;
}

unsigned int Peer::getCorrupt() {
	return corrupt;
}

bool Peer::hasChanged() {
	if (changed) {
		changed = false;
		return true;
	}
	return false;
}
