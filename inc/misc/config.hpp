#pragma once

#include <unordered_map>

class Config
{
	public:
		static void load (const std::string&);
		static std::string get (const std::string&);
		static int getInt (const std::string&);

	private:
		static bool checkValue (const std::string&, std::string&);
		static std::unordered_map<std::string, std::pair<std::string, bool>> vars;
};
