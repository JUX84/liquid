#pragma once

#include <string>

std::string responseHead ();
std::string response (const std::string&, bool gzip = false);
std::string error (const std::string&);
