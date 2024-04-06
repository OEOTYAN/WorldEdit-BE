#pragma once

#include "Globals.h"
namespace we {
int  download(std::string_view url, std::string_view outfilename);
bool downloadImage(std::string const& url);
} // namespace we
