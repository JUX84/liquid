#pragma once

#include <map>
#include "user.hpp"
#include "torrent.hpp"

typedef std::map<std::string, User> UserMap;
typedef std::map<std::string, Torrent> TorrentMap;

class Database {
	public:
		virtual void Connect() {};
		virtual void Disconnect() {};
		virtual void LoadUsers(UserMap&) {};
		virtual void LoadTorrents(TorrentMap&) {};
};
