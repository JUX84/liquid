#include "user.hpp"

User::User (unsigned int id) {
	this->id = id;
}

unsigned int* User::GetID () {
	return &this->id;
}
