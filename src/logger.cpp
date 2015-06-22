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
	if (level >= minLevel) {
			std::time_t time = std::time(nullptr);
			char timestr[9];
			if (std::strftime(timestr, sizeof(timestr), "%H:%M:%S", std::localtime(&time)))
				std::cout << "[" << timestr << "] ";
			if (level > 0)
				std::cout << file << ':' << line << " - " << levelNames[level] << " -- ";
			std::cout << message << '\n';
	}
}
