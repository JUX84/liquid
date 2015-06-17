#include <chrono>
#include <iostream>
#include "logger.hpp"
#include "config.hpp"
#include "peers.hpp"
#include "utility.hpp"

Peer::Peer(std::string hexIP, class User* u, bool seeding, unsigned long left, unsigned int fid, std::string client, std::string peerID) {
	LOG_INFO("Creating peer on torrent " + std::to_string(fid) + " using client " + client);
	user = u;
	total_stats = 0;
	stats = 0;
	this->left = left;
	this->seeding = seeding;
	completed = seeding;
	active = true;
	this->hexIP = hexIP;
	this->peerID = peerID;
	this->fid = fid;
	this->client = client;
	seedtime = 0;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	lastUpdate = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	this->speed = 0;
}

User* Peer::User() {
	return user;
}

const std::string& Peer::getPeerID() {
	return peerID;
}

const std::string& Peer::getHexIP() {
	return hexIP;
}

const std::string& Peer::getClient() {
	return client;
}

void Peer::updateStats (unsigned long stats, unsigned long left, long long now) {
	LOG_INFO("Updating peer stats ("  + std::to_string(stats) + ") on torrent " + std::to_string(fid));
	this->stats = stats - total_stats;
	total_stats = stats;
	this->left = left;
	if (now > lastUpdate)
		speed = (this->stats)/(now-lastUpdate);
	else
		speed = 0;
	seedtime += now - lastUpdate;
	lastUpdate = now;
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

void Peer::reset(long long now) {
	stats = seedtime = 0;
	lastUpdate = now;
}

unsigned int Peer::getFID() {
	return fid;
}

void Peer::snatched() {
	completed = true;
}

bool Peer::isSnatched() {
	return completed;
}

bool Peer::timedOut(long long now) {
	return (now - lastUpdate > Config::getInt("timeout"));
}

void Peer::setSeedtime(long long seedtime) {
	this->seedtime = seedtime;
}

long long Peer::getSeedtime() {
	return seedtime;
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
