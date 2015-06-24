#pragma once

#ifdef ENABLE_LOG

#define LOG_INIT(level) Logger::init(level)
#define LOG_INFO(message) Logger::write(Logger::Level::INFO, __FILENAME__, __LINE__, message)
#define LOG_WARNING(message) Logger::write(Logger::Level::WARNING, __FILENAME__, __LINE__, message)
#define LOG_ERROR(message) Logger::write(Logger::Level::ERROR, __FILENAME__, __LINE__, message)

#else

#define LOG_INIT(level)
#define LOG_INFO(message)
#define LOG_WARNING(message)
#define LOG_ERROR(message)

#endif

#include <string>

class Logger
{
	public:
		enum Level
		{
			INFO = 0,
			WARNING = 1,
			ERROR = 2
		};

		static void init(Level);
		static void write(Level, const char*, int, const std::string&);

	private:
		static Level minLevel;
		static const std::string levelNames[];
};
