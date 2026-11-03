#pragma once

#include <string>
#include <vector>

// Renders text as a multi-line block-letter ASCII banner.
// Returns one string per row of the banner; unsupported characters are
// rendered as blank space.
std::vector<std::string> renderAsciiArt(const std::string& text);
