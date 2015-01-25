#pragma once

#include <string>

std::string responseHead (bool = false);
std::string response (const std::string&, bool = false);
std::string error (const std::string&, bool = false);
