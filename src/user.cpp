#include <iostream>
#include "user.hpp"

User::User (unsigned int id) {
	std::cout << "Creating user " << std::to_string(id) << '\n';
	this->id = id;
}

unsigned int* User::getID () {
	return &this->id;
}

void User::updateStats(unsigned int dowloaded, unsigned int uploaded) {
	std::cout << "Updating stats of user " << std::to_string(id) << ": down (" << std::to_string(this->downloaded) << " -> " << std::to_string(this->downloaded+downloaded) << "), up (" << std::to_string(this->uploaded) << " -> " << std::to_string(this->uploaded+uploaded) << ")\n";
	this->downloaded += downloaded;
	this->uploaded += uploaded;
}

std::string User::record() {
	std::cout << "Recording user " << std::to_string(id) << " (down: " << downloaded << ", up: " << uploaded << ")\n";
	return "UPDATE users_main(Downloaded, Uploaded) SET Downloaded = Downloaded + "
		+ std::to_string(downloaded)
		+ ", Uploaded = Uploaded + "
		+ std::to_string(uploaded)
		+ " WHERE ID = "
		+ std::to_string(id);
	downloaded = uploaded = 0;
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
