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

void Logger::write(Logger::Level level, const char* file, int line, const std::string& message)
{
	if (level >= minLevel)
			std::cout << file << ':' << line << ": " << levelNames[level] << ": " << message << '\n';
}
