#pragma once

#ifdef ENABLE_LOG

#define LOG(level, message) Logger::write(level, message)

#else

#define LOG(level, message)

#endif

#include <string>

class Logger
{
	public:
		enum Level
		{
			INFO,
			WARNING,
			ERROR
		};

		static void init(Level);
		static void write(Level, const std::string&);

	private:
		static Level minLevel;
};
