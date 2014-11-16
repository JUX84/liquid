#include <algorithm>
#include <iostream>
#include <stdexcept>
#include "config.hpp"
#include "parser.hpp"

requirements Parser::required;

void Parser::init ()
{
	required.emplace("announce", std::forward_list<std::string>{"port","peer_id","info_hash","left"}); // init a set of required params in a request
	if (Config::get("type") == "private")
		required.at("announce").push_front("passkey");
}

std::string Parser::check (const request& req)
{
	try {
		for (auto it : required.at(req.at("action"))) {
			if (req.find(it) == req.end())
				return "missing param (" + it + ")";
		}
	} catch (const std::exception& e) {
		std::cerr << "Error in Parser::check -- action not found\n";
		return "missing action";
	}
	return "success";
}

request Parser::parse (const std::string& input)
{
	int input_length = input.length();
	if (input_length < 60)
		return request(); // too short
	request output; // first = params, second = headers
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
		output.emplace("passkey", input.substr(pos,32)); // Ocelot stated that substr 'exploded'. use for loop if necessary
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
		return request(); // malformed

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
					std::transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
					output.emplace("info_hash", hash);
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
	return output;
}
