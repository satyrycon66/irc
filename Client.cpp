#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"

Client::Client(int socket): _socket(socket), _authenticated(false){ return;}
// Client::Client(const std::string& username) : _username(username), _authenticated(false) {return }
Client::~Client(){ return ;}

int Client::getSocket() const { return _socket;}
const std::string& Client::getNick() const {return _nick;}
const std::string& Client::getUsername() const{    return _username;}
bool Client::isAuthenticated() const{ return _authenticated;}

void Client::setUsername(const std::string& username){    _username = username;}
void Client::setNick(const std::string& nick){    _nick = nick;}
void Client::authenticate(){    _authenticated = true;}

void Client::setUserMode(const std::string& nick, const std::string& mode,int fd) {
    _userModes.insert(std::pair<std::string, std::string >(nick, mode));
    std::string response = ":server MODE " + nick + " " + mode + "\r\n";
    send(fd, response.c_str(), response.length(), 0);
}

 std::string Client::getUserMode(const std::string& nick) const {
        std::map<std::string, std::string>::const_iterator it = _userModes.find(nick);
        if (it != _userModes.end()) {
            return it->second;
        }
        return ""; // Return empty string if user mode not found
    }


// Opérateur de comparaison ==
bool Client::operator==(const Client& other) const {
    return this->_username == other._username; // Comparaison basée sur le nom d'utilisateur par exemple
}

// Opérateur de comparaison !=
bool Client::operator!=(const Client& other) const {
    return !(*this == other); // Utilisation de l'opérateur == pour définir !=
}




// /server add IRC5 localhost/6667 -notls
// /set irc.server.IRC5.password secret
// /set irc.server.IRC5.nicks sim
// /set irc.server.IRC5.username simon
// /connect IRC5

// /server add IRC localhost/6667 -notls
// /set irc.server.IRC.password asd
// /set irc.server.IRC.nicks al
// /set irc.server.IRC.username alexis
// /connect IRC