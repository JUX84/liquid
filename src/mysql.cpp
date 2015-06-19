#include <iostream>
#include "logger.hpp"
#include "config.hpp"
#include "mysql.hpp"
#include "user.hpp"
#include "torrent.hpp"
#include "utility.hpp"

void MySQL::connect() {
	mysql = mysql_init(nullptr);
	if (mysql_real_connect(mysql, Config::get("DB_Host").c_str(), Config::get("DB_User").c_str(), Config::get("DB_Password").c_str(), Config::get("DB_DBName").c_str(), Config::getInt("DB_Port"), nullptr, 0) == nullptr) {
		LOG_ERROR("Couldn't connect to database");
	} else {
		LOG_INFO("Succesfully connected to database");
		std::string query = "TRUNCATE xbt_files_users";
		if (mysql_real_query(mysql, query.c_str(), query.size())) {
			LOG_ERROR("Couldn't truncate peers table");
			return;
		}
		query = "UPDATE torrents SET Leechers = 0, Seeders = 0";
		if (mysql_real_query(mysql, query.c_str(), query.size())) {
			LOG_ERROR("Couldn't reset torrent peers count");
			return;
		}
	}
}

void MySQL::disconnect() {
	std::string query = "TRUNCATE xbt_files_users";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't truncate peers table");
		return;
	}
	query = "UPDATE torrents SET Leechers = 0, Seeders = 0";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't reset torrent peers count");
		return;
	}
	LOG_WARNING("Disconnecting from database");
	mysql_free_result(result);
	mysql_close(mysql);
}

void MySQL::loadUsers(UserMap& usrMap) {
	std::string query = "SELECT ID, torrent_pass, can_leech FROM users_main";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load users database");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result)))
		usrMap.emplace(row[1], new User(std::stoul(row[0]), row[2] == std::string("1")));
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " users");

	// load tokens
	query = "SELECT u.torrent_pass, uf.TorrentID, UNIX_TIMESTAMP(uf.Time) FROM users_freeleeches AS uf LEFT JOIN users_main AS u ON uf.UserID = u.ID WHERE uf.Expired = '0'";
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
	query = "SELECT um.ID, um.torrent_pass, ir.IP FROM ip_restrictions AS ir INNER JOIN users_main as um ON um.ID = ir.UserID";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load IP restrictions addresses database");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		unsigned int userid = std::stoul(row[0]);
		std::string passkey = row[1];
		unsigned int ip = std::stoul(row[2]);
		try {
			User* u = usrMap.at("passkey");
			bool b = u->addIPRestriction(Utility::long2ip(ip), Config::getInt("max_ip"));
			if (!b)
				LOG_WARNING("Too many IP restrictions for user " + std::to_string(userid));
		} catch (const std::exception& e) {
			LOG_ERROR("No user (" + std::to_string(userid) + ") with passkey " + passkey);
		}
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " IP restrictions");
}

void MySQL::loadTorrents(TorrentMap& torMap) {
	std::string query = "SELECT ID, Size, info_hash, freetorrent, balance FROM torrents";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load torrents database");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		// TEMP FIX
		unsigned char free = 0;
		if (row[3] == std::string("1"))
			free = 100;
		else if (row[3] == std::string("2"))
			free = 50;
		//
		torMap.emplace(row[2], Torrent(std::stoul(row[0]), std::stoul(row[1]), free, std::stoul(row[4])));
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " torrents");
}

void MySQL::loadBannedIPs(std::unordered_set<std::string> &bannedIPs) {
	std::string query = "SELECT FromIP, ToIP FROM ip_bans";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load banned IP addresses database");
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

void MySQL::loadLeechStatus(LeechStatus &leechStatus) {
	std::string query = "SELECT `Data` FROM site_options WHERE `Option` = 'leech_status'";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load banned IP addresses database");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result)))
	{
		std::string data = row[0];
		leechStatus = (data == "freeleech" ? FREELEECH : NORMAL);
	}
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " banned IP addresses");
}

void MySQL::flush() {
	flushUsers();
	flushTorrents();
	flushPeers();
	flushTokens();
	flushSnatches();
}

void MySQL::flushUsers() {
	if (userRequests.size() == 0)
		return;
	std::string str = "INSERT INTO users_main(ID, Downloaded, Uploaded) VALUES ";
	for(const auto &it : userRequests) {
		if (str != "INSERT INTO users_main(ID, Downloaded, Uploaded) VALUES ")
			str += ", ";
		str += it;
	}
	str += " ON DUPLICATE KEY UPDATE Downloaded = Downloaded + VALUES(Downloaded), Uploaded = Uploaded + VALUES(Uploaded)";
	LOG_INFO("Flushing USERS sql records (" + std::to_string(userRequests.size()) + ")");
	if (mysql_real_query(mysql, str.c_str(), str.size())) {
		LOG_ERROR("Couldn't flush record (" + str + ")");
		return;
	}
	userRequests.clear();
}

void MySQL::flushTokens() {
	if (tokenRequests.size() == 0)
		return;
	std::string str = "INSERT INTO users_freeleeches(UserID, TorrentID, Downloaded, Expired) VALUES ";
	for(const auto &it : tokenRequests) {
		if (str != "INSERT INTO users_freeleeches(UserID, TorrentID, Downloaded, Expired) VALUES ")
			str += ", ";
		str += it;
	}
	str += " ON DUPLICATE KEY UPDATE Downloaded = Downloaded + VALUES(Downloaded), Expired = VALUES(Expired)";
	LOG_INFO("Flushing TOKENS sql records (" + std::to_string(tokenRequests.size()) + ")");
	if (mysql_real_query(mysql, str.c_str(), str.size())) {
		LOG_ERROR("Couldn't flush record (" + str + ")");
		return;
	}
	tokenRequests.clear();
}

void MySQL::flushTorrents() {
	if (torrentRequests.size() == 0)
		return;
	std::string str = "INSERT INTO torrents(ID, Seeders, Leechers, Snatched, balance) VALUES ";
	for(const auto &it : torrentRequests) {
		if (str != "INSERT INTO torrents(ID, Seeders, Leechers, Snatched, balance) VALUES ")
			str += ", ";
		str += it;
	}
	str += " ON DUPLICATE KEY UPDATE Seeders = VALUES(Seeders), Leechers = VALUES(Leechers), Snatched = Snatched + VALUES(Snatched), balance = VALUES(balance), last_action = NOW()";
	LOG_INFO("Flushing TORRENTS sql records (" + std::to_string(torrentRequests.size()) + ")");
	if (mysql_real_query(mysql, str.c_str(), str.size())) {
		LOG_ERROR("Couldn't flush record (" + str + ")");
		return;
	}
	torrentRequests.clear();
}

void MySQL::flushPeers() {
	if (peerRequests.size() == 0)
		return;
	std::string str = "INSERT INTO xbt_files_users (uid,active,completed,downloaded,uploaded,remaining,upspeed,downspeed,corrupt,timespent,useragent,peer_id,fid,ip) VALUES ";
	for(const auto &it : peerRequests) {
		if (str != "INSERT INTO xbt_files_users (uid,active,completed,downloaded,uploaded,remaining,upspeed,downspeed,corrupt,timespent,useragent,peer_id,fid,ip) VALUES ")
			str += ", ";
		str += it;
	}
	str += " ON DUPLICATE KEY UPDATE announced = announced + 1, downloaded = VALUES(downloaded), uploaded = VALUES(uploaded), timespent = timespent + VALUES(timespent), upspeed = VALUES(upspeed), downspeed = VALUES(downspeed), corrupt = VALUES(corrupt)";
	LOG_INFO("Flushing PEERS sql records (" + std::to_string(peerRequests.size()) + ")");
	if (mysql_real_query(mysql, str.c_str(), str.size())) {
		LOG_ERROR("Couldn't flush record (" + str + ")");
		return;
	}
	peerRequests.clear();
}

void MySQL::flushSnatches() {
	if (snatchRequests.size() == 0)
		return;
	std::string str = "INSERT INTO xbt_snatched (uid,tstamp,fid,IP) VALUES ";
	for(const auto &it : snatchRequests) {
		if (str != "INSERT INTO xbt_snatched (uid,tstamp,fid,IP) VALUES ")
			str += ", ";
		str += it;
	}
	LOG_INFO("Flushing SNATCHES sql records (" + std::to_string(snatchRequests.size()) + ")");
	if (mysql_real_query(mysql, str.c_str(), str.size())) {
		LOG_ERROR("Couldn't flush record (" + str + ")");
		return;
	}
	snatchRequests.clear();
}

void MySQL::recordUser(User* u) {
	if (u->hasChanged()) {
		unsigned long downloaded = u->getDownloaded();
		unsigned long uploaded = u->getUploaded();
		std::string ID = std::to_string(u->getID());
		std::string Downloaded = std::to_string(downloaded);
		std::string Uploaded = std::to_string(uploaded);
		LOG_INFO("Recording User " + ID + ": " + Utility::formatSize(downloaded) + " downloaded, " + Utility::formatSize(uploaded) + " uploaded");
		userRequests.push_back("(" + ID + ", " + Downloaded + ", " + Uploaded + ")");
		u->reset();
	}
}

void MySQL::recordToken(unsigned int userID, unsigned int torrentID, unsigned int downloaded, bool expired) {
	std::string UserID = std::to_string(userID);
	std::string TorrentID = std::to_string(torrentID);
	std::string Downloaded = std::to_string(downloaded);
	std::string Expired = (expired ? "TRUE" : "FALSE");
	LOG_INFO("Recording Token (UserID: " + UserID + ", TorrentID: " + TorrentID + ", " + Utility::formatSize(downloaded) + " downloaded, Expired: " + Expired + ")");
	tokenRequests.push_back("(" + UserID + ", " + TorrentID + ", " + Downloaded + ", " + Expired + ")");
}

void MySQL::recordTorrent(Torrent* t) {
	if (t->hasChanged()) {
		std::string ID = std::to_string(t->getID());
		std::string Seeders = std::to_string(t->getSeeders()->size());
		std::string Leechers = std::to_string(t->getLeechers()->size());
		std::string Snatches = std::to_string(t->getSnatches());
		std::string Balance = std::to_string(t->getBalance());
		LOG_INFO("Recording Torrent " + ID + ": " + Seeders + " Seeders, " + Leechers + " Leechers, " + Snatches + " new Snatches, Balance: " + Balance + "");
		torrentRequests.push_back("(" + ID + ", " + Seeders + ", " + Leechers + ", " + Snatches + ", " + Balance + ")");
		t->reset();
	}
}

void MySQL::recordPeer(Peer* p) {
	bool seeding = p->isSeeding();
	unsigned long total_stats,stats,left;
	unsigned int speed = p->getSpeed();
	left = p->getLeft();
	total_stats = p->getTotalStats();
	stats = p->getStats();
	std::string Left = std::to_string(left);
	std::string PeerID = p->getPeerID();
	std::string Timespent = std::to_string(p->getTimespent());
	std::string TorrentID = std::to_string(p->getTorrentID());
	LOG_INFO("Recording Peer " + PeerID + " on Torrent " + TorrentID + ": " + Utility::formatSize(left) + " left, " + Utility::formatSize(stats) + " " + (seeding ? "uploaded" : "downloaded") + " (" + Utility::formatSize(speed) + "/s), Timespent: " + Timespent + "");
	unsigned int downloaded,uploaded,total_downloaded,total_uploaded,up_speed,down_speed;
	if (seeding) {
		downloaded = 0;
		uploaded = stats;
		total_downloaded = 0;
		total_uploaded = total_stats;
		up_speed = speed;
		down_speed = 0;
	} else {
		downloaded = stats;
		uploaded = 0;
		total_downloaded = total_stats;
		total_uploaded = 0;
		up_speed = 0;
		down_speed = speed;
	}
	peerRequests.push_back("('" +
			std::to_string(p->getUser()->getID()) + "', " +
			(p->isActive() ? "1" : "0") + ", " +
			(p->isCompleted() ? "1" : "0") + ", " +
			"'" + std::to_string(total_downloaded) + "', " +
			"'" + std::to_string(total_uploaded) + "', " +
			"'" + Left + "', " +
			"'" + std::to_string(up_speed) + "', " +
			"'" + std::to_string(down_speed) + "', " +
			"'" + std::to_string(p->getCorrupt()) + "', " +
			"'" + Timespent + "', " +
			"'" + p->getClient() + "', " +
			"'" + PeerID + "', " +
			"'" + TorrentID + "', " +
			"'" + p->getIP() + "')");
	p->getUser()->updateStats(downloaded,uploaded);
}

void MySQL::recordSnatch(Peer* p, long long now) {
	std::string PeerID = p->getPeerID();
	std::string TorrentID = std::to_string(p->getTorrentID());
	LOG_INFO("Peer " + PeerID + " snatched torrent " + TorrentID);
	snatchRequests.push_back("('"
		+ std::to_string(p->getUser()->getID()) + "', " +
		"'" + std::to_string(now) + "', " +
		"'" + TorrentID + "', " +
		"'" + p->getIP() + "')");
}
