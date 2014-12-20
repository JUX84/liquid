#include <iostream>
#include "logger.hpp"

Logger::Level Logger::minLevel = Logger::Level::INFO;

const std::string Logger::levelNames[] = {
	"INFO",
	"WARNING",
	"ERROR"
};

void Logger::init(Logger::Level minLevel)
{
	Logger::minLevel = minLevel;
}

void Logger::write(Logger::Level level, const std::string& message)
{
	if (level >= minLevel) {
		if (level == Level::ERROR)
			std::cerr << levelNames[level] << ": " << message << '\n';
		else
			std::cout << levelNames[level] << ": " << message << '\n';
	}
}
