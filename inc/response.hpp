#pragma once

#include <string>

std::string responseHead (const bool& = false);
std::string response (const std::string&, const bool& = false);
std::string error (const std::string&, const bool& = false);
