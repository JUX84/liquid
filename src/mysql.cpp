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
		std::string query = "TRUNCATE xbt_files_users;";
		if (mysql_real_query(mysql, query.c_str(), query.size()))
			return;
	}
}

void MySQL::disconnect() {
	LOG_WARNING("Disconnecting from database");
	mysql_free_result(result);
	mysql_close(mysql);
}

void MySQL::loadUsers(UserMap& usrMap) {
	std::string query = "SELECT ID, torrent_pass, can_leech, visible FROM users_main";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load users database");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result)))
		usrMap.emplace(row[1], new User(std::stoul(row[0]), row[2] == std::string("1"), row[3] == std::string("1")));
	LOG_INFO("Loaded " + std::to_string(mysql_num_rows(result)) + " users");

	// load tokens
	query = "SELECT passkey, info_hash FROM users_freeleeches AS uf LEFT JOIN users AS u ON uf.UserID = u.ID JOIN torrents AS t ON uf.TorrentID = t.ID WHERE uf.Expired = '0'";
	if (mysql_real_query(mysql, query.c_str(), query.size())) {
		LOG_ERROR("Couldn't load user freeleeches");
		return;
	}
	result = mysql_use_result(mysql);
	while((row = mysql_fetch_row(result))) {
		try {
			usrMap.at(row[0])->addToken(row[1]);
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
	std::string query = "SELECT ID, Size, info_hash, freetorrent, Snatched FROM torrents";
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

void MySQL::record (std::string request) {
	LOG_INFO("Registering new sql record (" + request + ")");
	requests.push_front(request);
}

void MySQL::flush() {
	LOG_INFO("Flushing sql records (" + std::to_string(requests.size()) + ")");
	for(const auto &it : requests) {
		if (mysql_real_query(mysql, it.c_str(), it.size())) {
			LOG_ERROR("Couldn't flush record (" + it + ")");
			return;
		}
	}
	requests.clear();
}

void MySQL::recordUser(User* u) {
	std::string ID = std::to_string(u->getID());
	std::string Downloaded = std::to_string(u->getDownloaded());
	std::string Uploaded = std::to_string(u->getUploaded());
	LOG_INFO("Recording user " + ID + " stats: down (" + Downloaded + "), up (" + Uploaded + ")");                                                      
	record("UPDATE users_main SET Downloaded = Downloaded + "       
			+ Downloaded
			+ ", Uploaded = Uploaded + "
			+ Uploaded
			+ " WHERE ID = "
			+ ID);
	u->reset();
}

void MySQL::recordPeer(Peer* p, unsigned int left, long long now) {
	std::string Left = std::to_string(left);
	std::string PeerID = p->getPeerID();
	long long lastUpdate = p->getLastUpdate();
	unsigned long total_stats,stats;
       	total_stats = p->getTotalStats();
	stats = p->getStats();
	LOG_INFO("Recording peer stats (ID: " + PeerID + ", left: " + Left + ")");                                          
	unsigned int downloaded,uploaded,total_downloaded,total_uploaded = 0;
	if (p->isSeeding()) {
		downloaded = 0;
		uploaded = total_stats;
		total_downloaded = 0;
		total_uploaded = stats;
		p->setSeedtime(now - lastUpdate);
	} else {
		downloaded = total_stats;
		uploaded = 0;
		total_downloaded = stats;
		total_uploaded = 0;
	}
	record("INSERT INTO xbt_files_users(uid,active,completed,downloaded,uploaded,remaining,seedtime,useragent,peer_id,fid,ip) VALUES ('"
		+ std::to_string(p->User()->getID()) + "', 1, " +
		+ (p->isCompleted() ? "1" : "0") + ", " + 
		"'" + std::to_string(total_downloaded) + "', " +
		"'" + std::to_string(total_uploaded) + "', " +
		"'" + Left + "', " +
		"'" + std::to_string(p->getSeedtime()) + "', " +
		"'" + p->getClient() + "', " +
		"'" + PeerID + "', " +
		"'" + std::to_string(p->getFID()) + "', " +
		"'" + Utility::ip_hex_decode(p->getHexIP()) + "') ON DUPLICATE KEY UPDATE downloaded = VALUES(downloaded), uploaded = VALUES(uploaded), seedtime = seedtime + VALUES(seedtime)");
	p->reset(now);
	p->User()->updateStats(downloaded,uploaded,now);	
}

void MySQL::recordPeerSnatch(Peer* p, long long now) {
	std::string PeerID = p->getPeerID();
	std::string FID = std::to_string(p->getFID());
	LOG_INFO("Peer " + PeerID + " finished downloading torrent " + FID); 
	record("INSERT INTO xbt_snatched(uid,tstamp,fid,IP) VALUES ('"
		+ std::to_string(p->User()->getID()) + "', " +
		"'" + std::to_string(now) + "', " +
		"'" + FID + "', " +
		"'" + Utility::ip_hex_decode(p->getHexIP()) + "')");
}

void MySQL::recordPeerRemoval(Peer* p) {
	std::string PeerID = p->getPeerID();
	std::string FID = std::to_string(p->getFID());
	LOG_INFO("Removing peer " + PeerID + " on torrent " + FID);
	record("DELETE FROM xbt_files_users WHERE peer_id LIKE '"
			+ PeerID
			+ "' AND fid = "
			+ FID);
}

void MySQL::recordSnatch(Torrent* tor) {
	std::string FID = std::to_string(tor->getID());
	LOG_INFO("New snatch on torrent " + FID);
	record("UPDATE torrents SET Snatched = Snatched + 1");
}
