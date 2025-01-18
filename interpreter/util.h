//
// Created by jani on 1/18/25.
//

#ifndef UXSH_UTIL_H
#define UXSH_UTIL_H

#include <filesystem>
namespace fs = std::filesystem;

bool isSubDir(fs::path p, const fs::path& root);

#endif//UXSH_UTIL_H
