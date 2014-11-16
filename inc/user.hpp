#pragma once

#include <forward_list>
#include <string>

class User {
	private:
		std::forward_list<std::string> hexIP;
	public:
		void addHexIP(std::string hexIP);
		std::forward_list<std::string>* getHexIP();
};
