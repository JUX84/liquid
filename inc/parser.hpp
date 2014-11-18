#pragma once

#include <map>
#include <unordered_map>
#include <forward_list>
#include <string>

typedef std::pair<std::map<std::string, std::string>, std::forward_list<std::string>> Request;
typedef std::unordered_map<std::string, std::forward_list<std::string>> Requirements;

class Parser {
	private:
		static Requirements required;
	public:
		static void init ();
		static Request parse (const std::string&);
		static std::string check (const Request&);
};
