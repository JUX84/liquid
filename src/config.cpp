#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <iterator>
#include <utility>
#include <stdexcept>
#include "logger.hpp"
#include "config.hpp"

std::unordered_map<std::string, std::pair<std::string, bool>> Config::vars = {
//	{ "name",				{ "default value", is_integer} }
	{ "type",				{ "private", false } },
	{ "port",				{ "2710", true } },
	{ "timeout",				{ "3600", true } },
	{ "clear_peers_interval",		{ "600", true } },
	{ "read_buffer_size",			{ "512", true } },
	{ "max_request_size",			{ "4096", true } },
	{ "max_ip",				{ "5", true } },
	{ "max_numwant",			{ "50", true } },
	{ "default_numwant",			{ "20", true } },
	{ "updatekey",				{ "00000000000000000000000000000000", false } },
	{ "DB_Host",				{ "127.0.0.1", false } },
	{ "DB_User",				{ "db_user", false } },
	{ "DB_Password",			{ "db_password", false } },
	{ "DB_DBName",				{ "db_name", false } },
	{ "DB_Port",				{ "3306", true } },
};

std::string Config::get(const std::string& name)
{
	try {
		return vars.at(name).first;
	}
	catch (const std::exception& e) {
		LOG_ERROR("Error in Config::get(" + name + ")");
		return "";
	}
}

int Config::getInt(const std::string& name)
{
	try {
		return std::stoi(vars.at(name).first);
	}
	catch (const std::exception& e) {
		LOG_ERROR("Error in Config::getInt(" + name + ")");
		return -1;
	}
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

	if (!f) {
		LOG_WARNING("Couldn't open " + file + ". Aborted.");
		return;
	}

	while (!f.eof()) {
		std::getline(f, line);

		size_t start = line.find_first_not_of(whitespaces);
		if (line.empty() || start == std::string::npos || line[start] == '#') {
			++lineNumber;
			continue; // empty line or comment
		}

		size_t equalSign = line.find('=');
		if (equalSign == std::string::npos || equalSign == start || equalSign == line.size() - 1) {
			LOG_ERROR(std::to_string(lineNumber) + ": syntax error");
			continue;
		}

		std::string name = trim(line, start, equalSign - 1);
		if (vars.find(name) == vars.end()) {
			LOG_ERROR(std::to_string(lineNumber) + ": " + name + ": name not valid");
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
			LOG_ERROR(name + ": must be an integer");
			return false;
		}
	}
	else {
		if (value.front() == '"' && value.back() == '"') {
			value.erase(value.begin());
			value.pop_back();
		}
		else {
			LOG_ERROR(name + ": value must have leading and trailing \"");
			return false;
		}
	}

	return true;
}
