#pragma once

#include <unordered_map>

class Config
{
public:
	static void load(const std::string& file);
	static std::string get(const std::string& name);
	static int getInt(const std::string& name);

private:
	static bool checkValue(const std::string& name, std::string& value);

	static std::unordered_map<std::string, std::pair<std::string, bool>> vars;
};
