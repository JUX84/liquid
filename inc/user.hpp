#pragma once

#include <unordered_map>
#include <string>
#include "parser.hpp"

class User {
	private:
		std::unordered_map<std::string, std::string> hexIP;
	public:
		void addHexIP(const request&);
		std::unordered_map<std::string, std::string>* getHexIP();
};
