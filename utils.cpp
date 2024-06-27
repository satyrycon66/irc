#include "Server.hpp"

std::string toLower(const std::string& str) {
    std::string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = std::tolower(result[i]);
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