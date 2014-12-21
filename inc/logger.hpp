#pragma once

#ifdef ENABLE_LOG

#define LOG_INIT(level) Logger::init(level)
#define LOG(level, message) Logger::write(level, __FILENAME__, __LINE__, message)

#else

#define LOG_INIT(level)
#define LOG(level, message)

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
