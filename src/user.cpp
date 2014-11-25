#include "user.hpp"

User::User (unsigned int id) {
	this->id = id;
}

unsigned int* User::getID () {
	return &this->id;
}

void User::updateStats(unsigned int dowloaded, unsigned int uploaded) {
	this->downloaded += downloaded;
	this->uploaded += uploaded;
}

std::string User::record() {
	return "UPDATE users_main(Downloaded, Uploaded) SET Downloaded = Downloaded + "
		+ std::to_string(downloaded)
		+ ", Uploaded = Uploaded + "
		+ std::to_string(uploaded)
		+ " WHERE ID = "
		+ std::to_string(id);
	downloaded = uploaded = 0;
}
