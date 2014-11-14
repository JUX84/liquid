#pragma once

#include <string>

class User {
	private:
		std::string hex_ip;
	public:
		void set(std::string hex_ip);
		std::string get();
		bool isSet();
};
