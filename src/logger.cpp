#include <iostream>
#include "logger.hpp"

Logger::Level Logger::minLevel = Logger::Level::INFO;

void Logger::init(Logger::Level minLevel)
{
	Logger::minLevel = minLevel;
}

void Logger::write(Logger::Level level, const std::string& message)
{
	if (level >= minLevel) {
		if (level == Level::ERROR)
			std::cerr << message << '\n';
		else
			std::cout << message << '\n';
	}
}
