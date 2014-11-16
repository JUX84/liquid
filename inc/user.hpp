#pragma once

#include <string>

class User {
	private:
		std::string hexIP;
	public:
		void setHexIP(std::string hexIP);
		std::string getHexIP();
		bool isSetHexIP();
};
