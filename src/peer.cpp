#include <chrono>
#include <iostream>
#include "logger.hpp"
#include "config.hpp"
#include "peers.hpp"
#include "utility.hpp"

Peer::Peer(std::string IP, std::string port, class User* u, unsigned long announcedLeft, unsigned long announcedDownloaded, unsigned long announcedUploaded, unsigned int torrentID, std::string client, std::string peerID, bool ipv6) {
	LOG_INFO("Creating peer on torrent " + std::to_string(torrentID) + " using client " + client);
	user = u;
	totalDownloaded = announcedDownloaded;
	totalUploaded = announcedUploaded;
	downloaded = 0;
	uploaded = 0;
	left = announcedLeft;
	completed = left == 0;
	active = true;
	this->IP = IP;
	this->hexIPPort = Utility::ip_port_hex_encode(IP, port, ipv6);
	this->peerID = peerID;
	this->torrentID = torrentID;
	this->client = client;
	timespent = 0;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	lastUpdate = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
	downSpeed = 0;
	upSpeed = 0;
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

void Peer::updateStats (unsigned long announcedDownloaded, unsigned long announcedUploaded, unsigned long announcedLeft, unsigned int announcedCorrupt, long long now) {
	this->downloaded = announcedDownloaded - totalDownloaded;
	totalDownloaded = announcedDownloaded;
	this->uploaded = announcedUploaded - totalUploaded;
	totalUploaded = announcedUploaded;
	if (now > lastUpdate) {
		downSpeed = this->downloaded/(now-lastUpdate);
		upSpeed = this->uploaded/(now-lastUpdate);
	} else {
		downSpeed = 0;
		upSpeed = 0;
	}
	left = announcedLeft;
	corrupt = announcedCorrupt;
	timespent += now - lastUpdate;
	lastUpdate = now;
}

unsigned int Peer::getDownSpeed() {
	return downSpeed;
}

unsigned int Peer::getUpSpeed() {
	return upSpeed;
}

unsigned long Peer::getLeft() {
	return left;
}

long long Peer::getLastUpdate() {
	return lastUpdate;
}

void Peer::complete() {
	completed = true;
}

bool Peer::isCompleted() {
	return completed;
}

unsigned long Peer::getDownloaded() {
	return downloaded;
}

unsigned long Peer::getTotalDownloaded() {
	return totalDownloaded;
}

unsigned long Peer::getUploaded() {
	return uploaded;
}

unsigned long Peer::getTotalUploaded() {
	return totalUploaded;
}

unsigned int Peer::getTorrentID() {
	return torrentID;
}

bool Peer::timedOut(long long now) {
	return (now - lastUpdate) > Config::getInt("timeout");
}

unsigned long Peer::getTimespent() {
	unsigned long tmp = timespent;
	timespent = 0;
	return tmp;
}

bool Peer::isActive() {
	return active;
}

void Peer::inactive() {
	active = false;
}

unsigned int Peer::getCorrupt() {
	return corrupt;
}
