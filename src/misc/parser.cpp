#include <algorithm>
#include <stdexcept>
#include "misc/config.hpp"
#include "misc/logger.hpp"
#include "misc/parser.hpp"
#include "misc/utility.hpp"

Requirements Parser::required;

void Parser::init ()
{
	required.emplace("announce", std::forward_list<std::string>{"port","peer_id","left","user-agent","downloaded","uploaded"}); // init a set of required params in a request
	if (Config::get("type") == "private")
		required.at("announce").push_front("reqpasskey");
	required.emplace("scrape", std::forward_list<std::string>{});

	required.emplace("update", std::forward_list<std::string>{"type","reqpasskey"});
	required.emplace("change_passkey", std::forward_list<std::string>{"old_passkey", "new_passkey"});
	required.emplace("add_torrent", std::forward_list<std::string>{"id", "size"});
	required.emplace("delete_torrent", std::forward_list<std::string>{});
	required.emplace("update_torrent", std::forward_list<std::string>{"freetorrent"});
	required.emplace("add_user", std::forward_list<std::string>{"passkey", "id"});
	required.emplace("update_user", std::forward_list<std::string>{"passkey", "can_leech"});
	required.emplace("remove_user", std::forward_list<std::string>{"passkey"});
	required.emplace("remove_users", std::forward_list<std::string>{"passkeys"});
	required.emplace("add_token", std::forward_list<std::string>{"passkey"});
	required.emplace("remove_token", std::forward_list<std::string>{"passkey"});
	required.emplace("add_ban", std::forward_list<std::string>{"from", "to"});
	required.emplace("remove_ban", std::forward_list<std::string>{"from", "to"});
	required.emplace("add_ip_restriction", std::forward_list<std::string>{});
	required.emplace("remove_ip_restriction", std::forward_list<std::string>{});
	required.emplace("add_whitelist", std::forward_list<std::string>{"peer_id"});
	required.emplace("edit_whitelist", std::forward_list<std::string>{"old_peer_id", "new_peer_id"});
	required.emplace("remove_whitelist", std::forward_list<std::string>{"peer_id"});
	required.emplace("set_leech_status", std::forward_list<std::string>{});
}

std::string Parser::check (const Request& req)
{
	try {
		const std::string& action = req.at("action");
		for (const auto& it : required.at(action)) {
			if (req.find(it) == req.end())
				return "missing param (" + it + ")";
		}

		if (action == "update") {
			const std::string& type = req.at("type");
			try {
				for (const auto& it : required.at(type)) {
					if (req.find(it) == req.end())
						return "missing param (" + it + ")";
				}
			}
			catch (const std::exception& e) {
				LOG_ERROR("Error in Parser::check -- type for update not found");
				return "missing update type";
			}
		}
	} catch (const std::exception& e) {
		LOG_ERROR("Error in Parser::check -- action not found");
		return "missing action";
	}
	return "success";
}

std::pair<Request, std::forward_list<std::string>> Parser::parse (const std::string& input)
{
	int input_length = input.length();
	if (input_length < 60)
		return std::make_pair(Request(), std::forward_list<std::string>()); // too short
	Request output;
	int pos = 5; // skip 'GET /'
	/*
	Handles those types of request:
	Case 1: GET /passkey/action?params
	Case 2: GET /passkey?params
	Case 3: GET /action?params
	Case 4: GET /?params
	*/
	if ( input[pos] != '?' &&
			(input.substr(pos,32).find('/') == std::string::npos) &&
			(input.substr(pos,32).find('?') == std::string::npos)) { // Case 1, 2
		output.emplace("reqpasskey", input.substr(pos,32)); // Ocelot stated that substr 'exploded'. use for loop if necessary
		pos += 32;
	}
	if (input[pos] == '/') // Case 1
		++pos;
	int i;
	if ( input[pos] != '?' ) { // Case 1, 2, 3
		for (i=0; i < 8; ++i) { // longest action possible is announce
			if (input[pos+i] == '/' ||
				input[pos+i] == '?' ||
				input[pos+i] == '/')
				break;
		}
		output.emplace("action", input.substr(pos,i));
		pos += i;
	}
	if (input[pos] == '?') // Case 1,2,3,4
		++pos;
	else
		return std::make_pair(Request(), std::forward_list<std::string>()); // malformed

	std::forward_list<std::string> infoHashes;
	// ocelot-style parsing, should maybe replace with substr later
	std::string key;
	bool found_data = false;
	for (i=pos; i < input_length; ++i) {
		if (input[i] == '=') {
			key = input.substr(pos,i-pos);
			pos = ++i;
		} else if (input[i] == '&' || input[i] == ' ') {
			if (found_data) {
				if (key == "info_hash") {
					std::string hash = input.substr(pos, i-pos);
					hash = Utility::hex_to_bin(hash);
					infoHashes.push_front(hash);
				} else {
					output.emplace(key, input.substr(pos, i-pos));
				}
				pos = ++i;
			}
			found_data = false;
			if (input[pos-1] == ' ')
				break;
			key.clear();
		} else {
			found_data = true;
		}
	}
	pos += 10;
	for (i=pos; i < input_length; ++i) {
		if (input[i] == ':') {
			key = input.substr(pos, i-pos);
			i += 2;
			pos = i;
		} else if (input[i] == '\n' || input[i] == '\r') {
			if (found_data) {
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				if (key == "user-agent")
					output.emplace(key, input.substr(pos, i-pos));
				i += 2;
				pos = i;
			}
			found_data = false;
			key.clear();
		} else {
			found_data = true;
		}
	}
	return std::make_pair(output, infoHashes);
}
