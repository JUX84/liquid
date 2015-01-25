#include <iostream>
#include "logger.hpp"
#include "user.hpp"

User::User (unsigned int id, bool canLeech, bool isVisible) {
	this->id = id;
	this->downloaded = 0;
	this->uploaded = 0;
	this->canLeech = canLeech;
	this->isVisible = isVisible;
	auto duration = std::chrono::system_clock::now().time_since_epoch();
	this->lastUpdate = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
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

void User::updateStats(unsigned int dowloaded, unsigned int uploaded, long long now) {
	LOG_INFO("Updating stats of user " + std::to_string(id) + ": down (" + std::to_string(this->downloaded+downloaded) + "), up (" + std::to_string(this->uploaded+uploaded) + ")");
	this->downloaded += downloaded;
	this->uploaded += uploaded;
}

void User::addToken(const std::string& infoHash)
{
	if (std::find(tokens.begin(), tokens.end(), infoHash) == tokens.end())
		tokens.push_front(infoHash);
}

void User::removeToken(const std::string& infoHash)
{
	tokens.remove(infoHash);
}

bool User::hasToken(const std::string& infoHash) {
	if (std::find(tokens.begin(), tokens.end(), infoHash) != tokens.end())
		return true;
	return false;
}

bool User::canRecord(long long now) {
	bool b = (lastUpdate < (now - 300));
	lastUpdate = now;
	return b;
}
