#pragma once

#include <map>
#include <forward_list>
#include "user.hpp"
#include "torrent.hpp"

typedef std::map<std::string, User> UserMap;
typedef std::map<std::string, Torrent> TorrentMap;

class Database {
	protected:
		std::forward_list<std::string> requests;
	public:
		virtual void Connect() {};
		virtual void Disconnect() {};
		virtual void LoadUsers(UserMap&) {};
		virtual void LoadTorrents(TorrentMap&) {};
};
