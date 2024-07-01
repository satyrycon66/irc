#include "Server.hpp"

std::string toLower(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = std::tolower(result[i]);
    }
    return result;
}
std::string toUpper(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = std::toupper(result[i]);
    }
    return result;
}
std::string removeCRLF(const std::string& str) {
    std::string result = str;
    size_t pos = result.find("\r\n");
    if (pos != std::string::npos && pos == result.length() - 2) {
        result.erase(pos);
    }
    return result;
}
bool isValidMode(const std::string& modes) {
    // Check if modes starts with '+' or '-' followed by one of 'o', 'i', 't', 'k', 'l'
    if (modes.size() < 2 || (modes[0] != '+' && modes[0] != '-'&& modes[0] != '#')) {
        return false; // Modes should start with '+' or '-'
    }

    char modeChar = modes[1];
    const char validChars[] = {'o', 'i', 't', 'k', 'l'};
    const int numValidChars = sizeof(validChars) / sizeof(validChars[0]);

    // Check if the second character is one of 'o', 'i', 't', 'k', 'l'
    bool validSecondChar = false;
    for (int i = 0; i < numValidChars; ++i) {
        if (modeChar == validChars[i]) {
            validSecondChar = true;
            break;
        }
    }

    return validSecondChar;
}