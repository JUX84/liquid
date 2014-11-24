#pragma once

class User {
	private:
		unsigned int id;
		//unsigned long uploaded;
		//unsigned long downloaded;
	public:
		User (unsigned int id);
		unsigned int* GetID();
};
