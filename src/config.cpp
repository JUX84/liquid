#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iterator>
#include <utility>
#include <stdexcept>
#include "config.hpp"

std::unordered_map<std::string, std::pair<std::string, bool>> Config::vars = {
//	{ "name",				{ "default value", is_integer} }
	{ "type",				{ "public", false } },
	{ "port",				{ "48151", true } },
	{ "timeout",			{ "10", true } },
	{ "read_buffer_size",	{ "512", true } },
	{ "max_request_size",	{ "4096", true } },
};

std::string Config::get(const std::string& name)
{
	return vars.at(name).first;
}

int Config::getInt(const std::string& name)
{
	return std::stoi(get(name));
}

void Config::load(const std::string& file)
{
	std::ifstream f(file);
	std::string line;
	unsigned int lineNumber = 1;
	std::string whitespaces(" \t\v\f\r");
	auto trim = [&whitespaces](const std::string& str, size_t start, size_t end) -> std::string {
		start = str.find_first_not_of(whitespaces, start);
		end = str.find_last_not_of(whitespaces, end);
		return std::string(str, start, end - start + 1);
	};

	if (!f)
		throw std::runtime_error("Cannot open " + file);

	while (!f.eof()) {
		std::getline(f, line);
		if (line.empty())
			continue;

		size_t start = line.find_first_not_of(whitespaces);
		if (line[start] == '#')
			continue; // this line is a comment

		size_t equalSign = line.find('=');
		if (equalSign == std::string::npos || equalSign == start || equalSign == line.size() - 1) {
			std::cerr << std::to_string(lineNumber) + ": syntax error\n";
			continue;
		}

		std::string name = trim(line, start, equalSign - 1);
		if (vars.find(name) == vars.end()) {
			std::cerr << name + ": name not valid\n";
			continue;
		}
		std::string value = trim(line, equalSign + 1, line.size() - 1);

		if (checkValue(name, value))
			vars.at(name).first = value;

		++lineNumber;
	}
}

bool Config::checkValue(const std::string& name, std::string& value)
{
	if (vars.at(name).second) {
		try {
			std::stoi(value);
		}
		catch (const std::exception& e) {
			std::cerr << name << ": must be an integer\n";
			return false;
		}
	}
	else {
		if (value.front() == '"' && value.back() == '"') {
			value.erase(value.begin());
			value.pop_back();
		}
		else {
			std::cerr << name << ": value must have leading and trailing \"\n";
			return false;
		}
	}

	return true;
}