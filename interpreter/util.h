//
// Created by jani on 1/18/25.
//

#ifndef UXSH_UTIL_H
#define UXSH_UTIL_H

#include <filesystem>
namespace fs = std::filesystem;

bool isSubDir(fs::path p, const fs::path& root);

std::string performStandardCharEscaping(std::string& text);

#endif//UXSH_UTIL_H
