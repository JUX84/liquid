#include <algorithm>
#include "parser.hpp"

requirements Parser::required;

void Parser::init ()
{
	required.emplace("announce", std::forward_list<std::string>{"port","info_hash","left","numwant"});
}

std::string Parser::check (const request& req)
{
	if (req.find("action") == req.end() || required.find(req.at("action")) == required.end())
		return "missing action";
	for (auto it : required.at(req.at("action"))) {
		if (req.find(it) == req.end())
			return "missing param (" + it + ")";
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
	if ( input[pos] != '?' ) { // Case 1, 2, 3
		int i;
		for (i=0; i < 8; ++i) {
			if (input[pos+i] == '/')
				break;
		}
		output.emplace("action", input.substr(pos,i));
		pos += i;
		// TODO: scrape, update, report(?)
	}
	if (input[pos] == '?') // Case 1,2,3,4
		++pos;
	else
		return request(); // malformed
	std::string key, value;
	bool parsing_key = true;
	bool found_data = false;
	for (; pos < input_length; ++pos) {
		if (input[pos] == '=') {
			parsing_key = false;
		} else if (input[pos] == '&' || input[pos] == ' ') {
			if (found_data)
				output.emplace(key, value);
			found_data = false;
			parsing_key = true;
			if (input[pos] == ' ')
				break;
			key.clear();
			value.clear();
		} else {
			found_data = true;
			if (parsing_key)
				key.push_back(input[pos]);
			else
				value.push_back(input[pos]);
		}
	}
	pos += 10;
	for (; pos < input_length; ++pos) {
		if (input[pos] == ':') {
			if (!parsing_key) {
				while (input[pos+1] != '\n' && input[pos+1] != '\r')
					++pos;
			} else {
				parsing_key = false;
				++pos;
			}
		} else if (input[pos] == '\n' || input[pos] == '\r') {
			if (found_data) {
				std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				output.emplace(key, value);
			}
			found_data = false;
			parsing_key = true;
			key.clear();
			value.clear();
		} else {
			found_data = true;
			if (parsing_key)
				key.push_back(input[pos]);
			else
				value.push_back(input[pos]);
		}
	}
	return output;
}
