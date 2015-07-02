#include <thread>
#include "database/mysql.hpp"
#include "handler/torrent.hpp"
#include "handler/user.hpp"
#include "misc/config.hpp"
#include "misc/logger.hpp"
#include "misc/stats.hpp"
#include "misc/utility.hpp"

void MySQL::connect() {
	mysql = mysql_init(nullptr);
	if (mysql_real_connect(mysql, Config::get("DB_Host").c_str(), Config::get("DB_User").c_str(), Config::get("DB_Password").c_str(), Config::get("DB_DBName").c_str(), Config::getInt("DB_Port"), nullptr, 0) == nullptr) {
		LOG_ERROR("Couldn't connect to database");
	} else {
		LOG_INFO("Succesfully connected to database");
		reset();
	}
}

void MySQL::reset() {
	std::string query = "TRUNCATE " + Config::get("DB_Peers");
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't reset peers table");
		return;
	}
	query = "UPDATE " +
		Config::get("DB_Torrents") + " "
		"SET " +
		Config::get("DB_Torrents_Seeders") + " = 0, " +
		Config::get("DB_Torrents_Leechers") + " = 0";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't reset torrent peers count");
		return;
	}
}

void MySQL::disconnect() {
	reset();
	LOG_WARNING("Disconnecting from database");
	mysql_free_result(result);
	mysql_close(mysql);
}

void MySQL::loadUsers(UserMap& usrMap) {
	std::string query = "SELECT " +
		Config::get("DB_Users_ID") + ", " +
		Config::get("DB_Users_Passkey") + ", " +
		Config::get("DB_Users_Authorized") + " "
		"FROM users_main";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load users");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result)))
		usrMap.emplace(row[1], new User(std::stoul(row[0]), row[2] == std::string("1")));
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " users");

	// load tokens
	query = "SELECT "
		"users." + Config::get("DB_Users_Passkey") + ", "
		"tokens." + Config::get("DB_Tokens_TorrentID") + ", "
		"UNIX_TIMESTAMP(tokens." + Config::get("DB_Tokens_ExpirationTime") + ") "
		"FROM " +
		Config::get("DB_Tokens") + " AS tokens "
		"LEFT JOIN " +
		Config::get("DB_Users") + " AS users "
		"ON tokens." + Config::get("DB_Tokens_UserID") + " = " + Config::get("DB_Users_ID") + " "
		"WHERE tokens." + Config::get("DB_Tokens_Expired") + " = '0'";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load user freeleeches");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		try {
			usrMap.at(row[0])->addToken(std::stoul(row[1]), std::stoul(row[2]));
		} catch (const std::exception& e) {
			LOG_WARNING("Couldn't add freeleech token to user " + std::string(row[0]) + " (" + e.what() + ")");
		}
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " user tokens");

	// load ip restrictions
	query = "SELECT "
		"users." + Config::get("DB_Users_ID") + ", "
		"users." + Config::get("DB_Users_Passkey") + ", "
		"iprestrictions." + Config::get("DB_IPRestrictions_IP") + " "
		"FROM " +
		Config::get("DB_IPRestrictions") + " AS iprestrictions "
		"INNER JOIN " +
		Config::get("DB_Users") + " AS users "
		"ON users." + Config::get("DB_Users_ID") + " = iprestrictions." + Config::get("DB_IPRestrictions_UserID");
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load IP restrictions addresses");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		unsigned int userid = std::stoul(row[0]);
		std::string passkey = row[1];
		unsigned int ip = std::stoul(row[2]);
		try {
			User* u = usrMap.at(passkey);
			u->addIPRestriction(Utility::long2ip(ip));
		} catch (const std::exception& e) {
			LOG_ERROR("No user (" + std::to_string(userid) + ") with passkey " + passkey);
		}
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " IP restrictions");
}

void MySQL::loadTorrents(TorrentMap& torMap) {
	std::string query = "SELECT " +
		Config::get("DB_Torrents_ID") + ", " +
		Config::get("DB_Torrents_InfoHash") + ", " +
		Config::get("DB_Torrents_Freeleech") + ", " +
		Config::get("DB_Torrents_Snatches") + ", " +
		Config::get("DB_Torrents_Balance") + " "
		"FROM " +
		Config::get("DB_Torrents");
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load torrents");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		unsigned char free = 0;
		if (row[2] == std::string("1"))
			free = 100;
		else if (row[2] == std::string("2"))
			free = 50;
		torMap.emplace(row[1], Torrent(std::stoul(row[0]), free, std::stoul(row[3]), std::stoul(row[4])));
	}
	unsigned long torrentsCount = mysql_num_rows(result);
	Stats::setTorrents(torrentsCount);
	LOG_INFO("Loaded " + std::to_string(torrentsCount) + " torrents");
}

void MySQL::loadBannedIPs(std::unordered_set<std::string> &bannedIPs) {
	std::string query = "SELECT " +
		Config::get("DB_IPBans_FromIP") + ", " +
		Config::get("DB_IPBans_ToIP") + " "
		"FROM " +
		Config::get("DB_IPBans");
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load banned IP addresses");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		unsigned int from = std::stoul(row[0]);
		unsigned int to = std::stoul(row[1]);

		while (from != to)
			bannedIPs.emplace(Utility::long2ip(from++));
		bannedIPs.emplace(Utility::long2ip(from));
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " banned IP addresses");
}

void MySQL::loadClientWhitelist(std::list<std::string> &clientWhitelist) {
	std::string query = "SELECT " +
		Config::get("DB_Whitelist_PeerID") + " "
		"FROM " +
		Config::get("DB_Whitelist");
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load client whitelist");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		clientWhitelist.push_back(row[0]);
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " whitelisted clients");
}

void MySQL::loadLeechStatus(LeechStatus &leechStatus) {
	std::string query = "SELECT Value FROM " +
		Config::get("DB_SiteOptions") + " "
		"WHERE Key = '" + Config::get("DB_SiteOptions_LeechStatus") + "'";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load leech status");
		return;
	}
	result = mysql_use_result(mysql);
	std::string data = "NORMAL";
	while((row = mysql_fetch_row(result)))
	{
		data = row[0];
		leechStatus = (data == "freeleech" ? FREELEECH : NORMAL);
	}
	LOG_INFO("Loaded leech status: " + data);
}

void MySQL::flush() {
	flushUsers();
	flushTorrents();
	flushPeers();
	flushTokens();
	flushSnatches();
}

void MySQL::flushUsers() {
	//std::lock_guard<std::mutex> lock(userReqLock);
	if (userRequests.size() != 0) {
		std::string str = "INSERT INTO " +
			Config::get("DB_Users") + "(" +
			Config::get("DB_Users_ID") + ", " +
			Config::get("DB_Users_Downloaded") + ", " +
			Config::get("DB_Users_Uploaded") + ") "
			"VALUES ";
		bool first = true;
		for(const auto &it : userRequests) {
			if (!first)
				str += ", ";
			str += it;
			first = false;
		}
		str += " ON DUPLICATE KEY UPDATE " +
			Config::get("DB_Users_Downloaded") + " = " + Config::get("DB_Users_Downloaded") + " + VALUES(" + Config::get("DB_Users_Downloaded") + "), " +
			Config::get("DB_Users_Uploaded") + " = " + Config::get("DB_Users_Uploaded") + " + VALUES(" + Config::get("DB_Users_Uploaded") + ")";
		LOG_INFO("Flushing USERS sql records (" + std::to_string(userRequests.size()) + ")");
		userRecords.push_back(str);
		userRequests.clear();
	}
	if (!usersFlushing && userRecords.size() != 0) {
		std::thread thread(&MySQL::doFlushUsers, this);
		thread.detach();
	}
}

void MySQL::doFlushUsers() {
	std::lock_guard<std::mutex> lock(sqlLock);
	usersFlushing = true;
	for (const auto &it : userRecords) {
		if (mysql_real_query(mysql, it.c_str(), it.size())) {
			LOG_ERROR("Couldn't flush record (" + it + ")");
			return;
		}
	}
	userRecords.clear();
	usersFlushing = false;
}

void MySQL::flushTokens() {
	//std::lock_guard<std::mutex> lock(tokenReqLock);
	if (tokenRequests.size() != 0) {
		std::string str = "INSERT INTO " +
			Config::get("DB_Tokens") + "(" +
			Config::get("DB_Tokens_UserID") + ", " +
			Config::get("DB_Tokens_TorrentID") + ", " +
			Config::get("DB_Tokens_Downloaded") + ", " +
			Config::get("DB_Tokens_Expired") + ") "
			"VALUES ";
		bool first = true;
		for(const auto &it : tokenRequests) {
			if (!first)
				str += ", ";
			str += it;
			first = false;
		}
		str += " ON DUPLICATE KEY UPDATE " +
			Config::get("DB_Tokens_Downloaded") + " = " + Config::get("DB_Tokens_Downloaded") + " + VALUES(" + Config::get("DB_Tokens_Downloaded") + "), " +
			Config::get("DB_Tokens_Expired") + " = VALUES(" + Config::get("DB_Tokens_Expired") + ")";
		LOG_INFO("Flushing TOKENS sql records (" + std::to_string(tokenRequests.size()) + ")");
		tokenRecords.push_back(str);
		tokenRequests.clear();
	}
	if (!tokensFlushing && tokenRecords.size() != 0) {
		std::thread thread(&MySQL::doFlushTokens, this);
		thread.detach();
	}
}

void MySQL::doFlushTokens() {
	std::lock_guard<std::mutex> lock(sqlLock);
	tokensFlushing = true;
	for (const auto &it : tokenRecords) {
		if (mysql_real_query(mysql, it.c_str(), it.size())) {
			LOG_ERROR("Couldn't flush record (" + it + ")");
			return;
		}
	}
	tokenRecords.clear();
	tokensFlushing = false;
}

void MySQL::flushTorrents() {
	//std::lock_guard<std::mutex> lock(torrentReqLock);
	if (torrentRequests.size() != 0) {
		std::string str = "INSERT INTO " +
			Config::get("DB_Torrents") + "(" +
			Config::get("DB_Torrents_ID") + ", " +
			Config::get("DB_Torrents_Seeders") + ", " +
			Config::get("DB_Torrents_Leechers") + ", " +
			Config::get("DB_Torrents_Snatches") + ", " +
			Config::get("DB_Torrents_Balance") + ") "
			"VALUES ";
		bool first = true;
		for(const auto &it : torrentRequests) {
			if (!first)
				str += ", ";
			str += it;
			first = false;
		}
		str += " ON DUPLICATE KEY UPDATE " +
			Config::get("DB_Torrents_Seeders") + " = VALUES(" + Config::get("DB_Torrents_Seeders") + "), " +
			Config::get("DB_Torrents_Leechers") + " = VALUES(" + Config::get("DB_Torrents_Leechers") + "), " +
			Config::get("DB_Torrents_Snatches") + " = VALUES(" + Config::get("DB_Torrents_Snatches") + "), " +
			Config::get("DB_Torrents_Balance") + " = VALUES(" + Config::get("DB_Torrents_Balance") + "), " +
			Config::get("DB_Torrents_LastAction") + " = NOW()";
		LOG_INFO("Flushing TORRENTS sql records (" + std::to_string(torrentRequests.size()) + ")");
		torrentRecords.push_back(str);
		torrentRequests.clear();
	}
	if (!torrentsFlushing && torrentRecords.size() != 0) {
		std::thread thread(&MySQL::doFlushTorrents, this);
		thread.detach();
	}
}

void MySQL::doFlushTorrents() {
	std::lock_guard<std::mutex> lock(sqlLock);
	torrentsFlushing = true;
	for (const auto &it : torrentRecords) {
		if (mysql_real_query(mysql, it.c_str(), it.size())) {
			LOG_ERROR("Couldn't flush record (" + it + ")");
			return;
		}
	}
	torrentRecords.clear();
	torrentsFlushing = false;
}

void MySQL::flushPeers() {
	//std::lock_guard<std::mutex> lock(peerReqLock);
	if (peerRequests.size() != 0) {
		std::string str = "INSERT INTO " +
			Config::get("DB_Peers") + "(" +
			Config::get("DB_Peers_UserID") + ", " +
			Config::get("DB_Peers_Active") + ", " +
			Config::get("DB_Peers_AnnouncesCount") + ", " +
			Config::get("DB_Peers_Completed") + ", " +
			Config::get("DB_Peers_Downloaded") + ", " +
			Config::get("DB_Peers_Uploaded") + ", " +
			Config::get("DB_Peers_Left") + ", " +
			Config::get("DB_Peers_UpSpeed") + ", " +
			Config::get("DB_Peers_DownSpeed") + ", " +
			Config::get("DB_Peers_Corrupt") + ", " +
			Config::get("DB_Peers_Timespent") + ", " +
			Config::get("DB_Peers_UserAgent") + ", " +
			Config::get("DB_Peers_PeerID") + ", " +
			Config::get("DB_Peers_TorrentID") + ", " +
			Config::get("DB_Peers_IP") + ", " +
			Config::get("DB_Peers_LastAction") + ") " +
			"VALUES ";
		bool first = true;
		for(const auto &it : peerRequests) {
			if (!first)
				str += ", ";
			str += it;
			first = false;
		}
		str += " ON DUPLICATE KEY UPDATE " +
			Config::get("DB_Peers_AnnouncesCount") + " = " + Config::get("DB_Peers_AnnouncesCount") + " + 1, " +
			Config::get("DB_Peers_Downloaded") + " = VALUES(" + Config::get("DB_Peers_Downloaded") + "), " +
			Config::get("DB_Peers_Uploaded") + " = VALUES(" + Config::get("DB_Peers_Uploaded") + "), " +
			Config::get("DB_Peers_Timespent") + " = " + Config::get("DB_Peers_Timespent") + " + VALUES(" + Config::get("DB_Peers_Timespent") + "), " +
			Config::get("DB_Peers_UpSpeed") + " = VALUES(" + Config::get("DB_Peers_UpSpeed") + "), " +
			Config::get("DB_Peers_DownSpeed") + " = VALUES(" + Config::get("DB_Peers_DownSpeed") + "), " +
			Config::get("DB_Peers_Corrupt") + " = VALUES(" + Config::get("DB_Peers_Corrupt") + "), " +
			Config::get("DB_Peers_LastAction") + " = VALUES(" + Config::get("DB_Peers_LastAction") + ")";
		LOG_INFO("Flushing PEERS sql records (" + std::to_string(peerRequests.size()) + ")");
		peerRecords.push_back(str);
		peerRequests.clear();
	}
	if (!peersFlushing && peerRecords.size() != 0) {
		std::thread thread(&MySQL::doFlushPeers, this);
		thread.detach();
	}
}

void MySQL::doFlushPeers() {
	std::lock_guard<std::mutex> lock(sqlLock);
	peersFlushing = true;
	for (const auto &it : peerRecords) {
		if (mysql_real_query(mysql, it.c_str(), it.size())) {
			LOG_ERROR("Couldn't flush record (" + it + ")");
			return;
		}
	}
	peerRecords.clear();
	peersFlushing = false;
}

void MySQL::flushSnatches() {
	//std::lock_guard<std::mutex> lock(snatchReqLock);
	if (snatchRequests.size() != 0) {
		std::string str = "INSERT IGNORE INTO " +
			Config::get("DB_Snatches") + "(" +
			Config::get("DB_Snatches_UserID") + ", " +
			Config::get("DB_Snatches_TimeStamp") + ", " +
			Config::get("DB_Snatches_TorrentID") + ", " +
			Config::get("DB_Snatches_IP") + ") " +
			"VALUES ";
		bool first = true;
		for(const auto &it : snatchRequests) {
			if (!first)
				str += ", ";
			str += it;
			first = false;
		}
		LOG_INFO("Flushing SNATCHES sql records (" + std::to_string(snatchRequests.size()) + ")");
		snatchRecords.push_back(str);
		snatchRequests.clear();
	}
	if (!snatchesFlushing && snatchRecords.size() != 0) {
		std::thread thread(&MySQL::doFlushSnatches, this);
		thread.detach();
	}
}

void MySQL::doFlushSnatches() {
	std::lock_guard<std::mutex> lock(sqlLock);
	snatchesFlushing = true;
	for (const auto &it : snatchRecords) {
		if (mysql_real_query(mysql, it.c_str(), it.size())) {
			LOG_ERROR("Couldn't flush record (" + it + ")");
			return;
		}
	}
	snatchRecords.clear();
	snatchesFlushing = false;
}

void MySQL::recordUser(User* u) {
	if (u->hasChanged()) {
		unsigned long downloaded = u->getDownloaded();
		unsigned long uploaded = u->getUploaded();
		std::string ID = std::to_string(u->getID());
		std::string Downloaded = std::to_string(downloaded);
		std::string Uploaded = std::to_string(uploaded);
		LOG_INFO("Recording User " + ID + ": " + Downloaded + " B downloaded, " + Uploaded + " B uploaded");
		//std::lock_guard<std::mutex> lock(userReqLock);
		userRequests.push_back("(" + ID + ", " + Downloaded + ", " + Uploaded + ")");
		u->reset();
	}
}

void MySQL::recordToken(unsigned int userID, unsigned int torrentID, unsigned int downloaded, bool expired) {
	std::string UserID = std::to_string(userID);
	std::string TorrentID = std::to_string(torrentID);
	std::string Downloaded = std::to_string(downloaded);
	std::string Expired = (expired ? "TRUE" : "FALSE");
	LOG_INFO("Recording Token (UserID: " + UserID + ", TorrentID: " + TorrentID + ", " + Downloaded + " B downloaded, Expired: " + Expired + ")");
	//std::lock_guard<std::mutex> lock(tokenReqLock);
	tokenRequests.push_back("(" + UserID + ", " + TorrentID + ", " + Downloaded + ", " + Expired + ")");
}

void MySQL::recordTorrent(Torrent* t) {
	std::string ID = std::to_string(t->getID());
	std::string Seeders = std::to_string(t->getSeeders()->size() + t->getSeeders6()->size());
	std::string Leechers = std::to_string(t->getLeechers()->size() + t->getLeechers6()->size());
	std::string Snatches = std::to_string(t->getSnatches());
	std::string Balance = std::to_string(t->getBalance());
	LOG_INFO("Recording Torrent " + ID + ": " + Seeders + " Seeders, " + Leechers + " Leechers, " + Snatches + " Snatches, Balance: " + Balance + "");
	//std::lock_guard<std::mutex> lock(torrentReqLock);
	torrentRequests.push_back("(" + ID + ", " + Seeders + ", " + Leechers + ", " + Snatches + ", " + Balance + ")");
}

void MySQL::recordPeer(Peer* p) {
	unsigned long downloaded, uploaded, totalDownloaded, totalUploaded, left;
	unsigned int downSpeed,upSpeed;
	downloaded = p->getDownloaded();
	uploaded = p->getUploaded();
	totalDownloaded = p->getTotalDownloaded();
	totalUploaded = p->getTotalUploaded();
	downSpeed = p->getDownSpeed();
	std::string DownSpeed = std::to_string(downSpeed);
	upSpeed = p->getUpSpeed();
	std::string UpSpeed = std::to_string(upSpeed);
	left = p->getLeft();
	std::string Left = std::to_string(left);
	std::string PeerID = p->getPeerID();
	std::string Timespent = std::to_string(p->getTimespent());
	std::string TorrentID = std::to_string(p->getTorrentID());
	LOG_INFO("Recording Peer " + PeerID + " on Torrent " + TorrentID + ": " + Left + " B left, " + std::to_string(downloaded) + " B downloaded (" + DownSpeed + " B/s), " + std::to_string(uploaded) + " B uploaded " + " (" + UpSpeed + " B/s), Timespent: " + Timespent);
	//std::lock_guard<std::mutex> lock(peerReqLock);
	peerRequests.push_back("('" +
			(Config::get("type") == "private" ? std::to_string(p->getUser()->getID()) : "0") + "', " +
			(p->isActive() ? "1" : "0") + ", 1, " +
			(p->isCompleted() ? "1" : "0") + ", " +
			"'" + std::to_string(totalDownloaded) + "', " +
			"'" + std::to_string(totalUploaded) + "', " +
			"'" + Left + "', " +
			"'" + UpSpeed + "', " +
			"'" + DownSpeed + "', " +
			"'" + std::to_string(p->getCorrupt()) + "', " +
			"'" + Timespent + "', " +
			"'" + p->getClient() + "', " +
			"'" + PeerID + "', " +
			"'" + TorrentID + "', " +
			"'" + p->getIP() + "', UNIX_TIMESTAMP())");
}

void MySQL::recordSnatch(Peer* p, long long now) {
	std::string PeerID = p->getPeerID();
	std::string TorrentID = std::to_string(p->getTorrentID());
	LOG_INFO("Peer " + PeerID + " snatched torrent " + TorrentID);
	//std::lock_guard<std::mutex> lock(snatchReqLock);
	snatchRequests.push_back("('" +
		(Config::get("type") == "private" ? std::to_string(p->getUser()->getID()) : "0") + "', " +
		"'" + std::to_string(now) + "', " +
		"'" + TorrentID + "', " +
		"'" + p->getIP() + "')");
}
