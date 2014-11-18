#pragma once

#include <map>
#include <unordered_map>
#include <forward_list>
#include <string>

typedef std::map<std::string, std::string> infos;
typedef std::pair<infos, std::forward_list<std::string>> request;
typedef std::unordered_map<std::string, std::forward_list<std::string>> requirements;

class Parser {
	private:
		static requirements required;
	public:
		static void init ();
		static request parse (const std::string& input);
		static std::string check (const request& input);
};
