#include <chrono>
#include <iostream>
#include "logger.hpp"
#include "config.hpp"
#include "peers.hpp"
#include "utility.hpp"

Peer::Peer(std::string hexIP, class User* u, bool seeding, unsigned int fid, std::string client, std::string peerID) {
	LOG_INFO("Creating peer on torrent " + std::to_string(fid) + " using client " + client);
	this->hexIP = hexIP;
	user = u;
	this->seeding = seeding;
	completed = seeding;
	this->fid = fid;
	this->client = client;
	this->peerID = peerID;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	lastUpdate = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	seedtime = 0;
	active = true;
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

void Peer::updateStats (unsigned long stats, long long now) {
	LOG_INFO("Updating peer stats ("  + std::to_string(stats) + ") on torrent " + std::to_string(fid));
	this->stats = stats - total_stats;
	total_stats = stats;
	seedtime += now - lastUpdate;
	lastUpdate = now;
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
