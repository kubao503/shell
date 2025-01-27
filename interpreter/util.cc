//
// Created by jani on 1/18/25.
//

#include "util.h"
#include "interpreter.hh"
#include <filesystem>
#include <unordered_map>


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
const std::unordered_map<char, char> ESCAPE_MAP = {
    {'\\', '\\'},
    {'a', '\a'},
    {'b', '\b'},
    {'f', '\f'},
    {'n', '\n'},
    {'r', '\r'},
    {'t', '\t'},
    {'v', '\v'}
};
std::string performStandardCharEscaping(std::string &text) {
    std::string escapedText;
    bool inEscape = false;
    for(auto c : text) {
        if(inEscape) {
            auto replacement = ESCAPE_MAP.find(c);
            if (replacement == ESCAPE_MAP.end()) throw InterpreterError("Incorrect escaping");
            escapedText += replacement->second;
            inEscape = false;
        }
        else {
            if(c == '\\') inEscape = true;
            else escapedText += c;
        }
    }
    return escapedText;
}
