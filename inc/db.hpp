#pragma once

#include <map>
#include "user.hpp"
#include "torrent.hpp"

typedef std::map<std::string, User> userMap;
typedef std::map<std::string, Torrent> torrentMap;

class Database {
	public:
		virtual void Connect(std::string) {};
		virtual void Disconnect() {};
		virtual void LoadUsers(userMap&) {};
		virtual void LoadTorrents(torrentMap&) {};
};
