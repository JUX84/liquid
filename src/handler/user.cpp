#include <chrono>
#include "handler/user.hpp"
#include "misc/logger.hpp"

User::User (unsigned int id, bool authorized) {
	this->id = id;
	downloaded = 0;
	uploaded = 0;
	this->authorized = authorized;
}

unsigned int User::getID () {
	return id;
}

unsigned long User::getDownloaded() {
	return downloaded;
}

unsigned long User::getUploaded() {
	return uploaded;
}

void User::reset() {
	downloaded = uploaded = 0;
}

void User::updateStats(unsigned int downloaded, unsigned int uploaded) {
	this->downloaded += downloaded;
	this->uploaded += uploaded;
}

void User::addToken(unsigned int tid, long long time)
{
	try {
		tokens.at(tid) = time;
	} catch (const std::exception& e) {
		tokens.emplace(tid, time);
	}
}

void User::removeToken(unsigned int tid)
{
	tokens.erase(tid);
}

bool User::hasToken(unsigned int tid) {
	return tokens.find(tid) != tokens.end();
}

bool User::isTokenExpired(unsigned int tid) {
	try {
		auto duration = std::chrono::system_clock::now().time_since_epoch();
		long long now = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
		return ((now - tokens.at(tid)) < (3600 * 24)); // tokens expire after 24h or upon completion
	} catch (const std::exception& e) {
		LOG_ERROR("A token was found but an error occured while checking it (UserID: " + std::to_string(id) + ", TorrentID: " + std::to_string(tid) + ")");
		return false;
	}
}

void User::addIPRestriction(std::string ip) {
	IPRestrictions.emplace(ip);
}

void User::removeIPRestriction(std::string ip) {
	IPRestrictions.erase(ip);
}

bool User::isRestricted(std::string ip) {
	if (IPRestrictions.size() == 0)
		return false;
	for (const auto &it : IPRestrictions) {
		if (ip == it)
			return false;
	}
	return true;
}

bool User::isAuthorized() {
	return authorized;
}

void User::setAuthorized(bool authorized) {
	this->authorized = authorized;
}

bool User::hasChanged() {
	return (downloaded != 0) || (uploaded != 0);
}
