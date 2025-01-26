//
// Created by jani on 1/18/25.
//

#include "util.h"
#include <cstring>
#include <filesystem>
#include <iostream>


namespace fs = std::filesystem;

bool isSubDir(fs::path p, const fs::path& root)
{
    fs::path systemRoot("/");
    while(p != systemRoot) {
        if(p == root) {
             return true;
        }
        p = p.parent_path();
    }
    return false;
}
void handleErrnoExit(std::string text) {
    std::cerr << text << " with error: " + std::string(strerror(errno));
    exit(errno);
}
